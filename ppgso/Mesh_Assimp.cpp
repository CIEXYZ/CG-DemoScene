#include <glm/glm.hpp>
#include <sstream>
#include <filesystem>

#include "Mesh_Assimp.h"

ppgso::Mesh_Assimp::Mesh_Assimp(const std::string &obj_file) {
#ifdef DEBBUG_MODE
    std::cout << "Using ASSIMP Loader!" << std::endl;
#endif

    // Store base path for later use
    basePath = std::filesystem::path(obj_file).parent_path().string();

    // Load the scene - importer is now a member so scene stays valid
    scene = importer.ReadFile(obj_file, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::stringstream msg;
        msg << importer.GetErrorString() << std::endl << "Failed to load OBJ file " << obj_file << "!" << std::endl;
        throw std::runtime_error(msg.str());
    }

    std::cout << "Model loaded successfully: " << obj_file << std::endl;
    std::cout << "Number of materials: " << scene->mNumMaterials << std::endl;
    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;

    processNode(scene->mRootNode, scene);

    // Load materials and their texture info
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        aiMaterial* material = scene->mMaterials[i];

        // Get material name
        aiString materialName;
        material->Get(AI_MATKEY_NAME, materialName);
        std::cout << "Material " << i << ": " << materialName.C_Str() << std::endl;

        // Diffuse color
        aiColor3D diffuseColor(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
        this->diffuse.emplace_back(diffuseColor.r, diffuseColor.g, diffuseColor.b);

        // Ambient color
        aiColor3D ambientColor(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
        this->ambient.emplace_back(ambientColor.r, ambientColor.g, ambientColor.b);

        // Specular color
        aiColor3D specularColor(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
        this->specular.emplace_back(specularColor.r, specularColor.g, specularColor.b);

        // Check for textures
        MaterialTextures matTextures;

        // Diffuse texture
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString texPath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
            std::cout << "  Diffuse texture: " << texPath.C_Str() << std::endl;
        }

        // Normal texture
        if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
            aiString texPath;
            material->GetTexture(aiTextureType_NORMALS, 0, &texPath);
            std::cout << "  Normal texture: " << texPath.C_Str() << std::endl;
        }

        // Specular texture
        if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
            aiString texPath;
            material->GetTexture(aiTextureType_SPECULAR, 0, &texPath);
            std::cout << "  Specular texture: " << texPath.C_Str() << std::endl;
        }

        textures.push_back(std::move(matTextures));
    }
}

ppgso::Mesh_Assimp::~Mesh_Assimp() {
    for(auto& buffer : buffers) {
        glDeleteBuffers(1, &buffer.ibo);
        glDeleteBuffers(1, &buffer.nbo);
        glDeleteBuffers(1, &buffer.tbo);
        glDeleteBuffers(1, &buffer.vbo);
        glDeleteVertexArrays(1, &buffer.vao);
    }
}

void ppgso::Mesh_Assimp::processNode(aiNode *node, const aiScene *pScene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *mesh = pScene->mMeshes[node->mMeshes[i]];
        processMesh(mesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], pScene);
    }
}

void ppgso::Mesh_Assimp::processMesh(aiMesh *mesh) {
    gl_buffer buffer;

    // Store material index for this mesh
    buffer.materialIndex = mesh->mMaterialIndex;

    std::cout << "Processing mesh with material index: " << buffer.materialIndex << std::endl;

    // Process vertices
    if (mesh->HasPositions()) {
        // Generate a vertex array object
        glGenVertexArrays(1, &buffer.vao);
        glBindVertexArray(buffer.vao);

        // Extract vertex positions from aiMesh and upload to GPU
        glGenBuffers(1, &buffer.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(aiVector3D), mesh->mVertices, GL_STATIC_DRAW);
        // Enable and set up vertex attribute pointer for positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    // Process texture coordinates
    if (mesh->HasTextureCoords(0)) {
        std::vector<aiVector2D> textureCoords;
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            aiVector3D texCoord = mesh->mTextureCoords[0][i]; // Assuming single texture channel (index 0)
            textureCoords.push_back(aiVector2D(texCoord.x, texCoord.y));
        }

        glGenBuffers(1, &buffer.tbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.tbo);
        glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(aiVector2D), textureCoords.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }


    // Process normals
    if (mesh->HasNormals()) {
        std::vector<aiVector3D> normals;
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            aiVector3D normal = mesh->mNormals[i];
            normals.push_back(normal);
        }

        glGenBuffers(1, &buffer.nbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.nbo);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(aiVector3D), normals.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }


    // Process indices
    if (mesh->HasFaces()) {
        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Upload indices to GPU
        glGenBuffers(1, &buffer.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        buffer.size = static_cast<GLsizei>(indices.size());
    }

    buffers.push_back(buffer);
}

void ppgso::Mesh_Assimp::render() {
    for (auto &buffer : buffers) {
        // Draw object
        glBindVertexArray(buffer.vao);
        glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);
    }
}

void ppgso::Mesh_Assimp::renderWithMaterials(std::function<void(unsigned int)> setMaterialCallback) {
    for (auto &buffer : buffers) {
        // Set material for this mesh part
        setMaterialCallback(buffer.materialIndex);

        // Draw object
        glBindVertexArray(buffer.vao);
        glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);
    }
}

std::string ppgso::Mesh_Assimp::getTexturePath(unsigned int materialIndex, aiTextureType type) const {
    if (!scene || materialIndex >= scene->mNumMaterials) {
        return "";
    }

    aiMaterial* material = scene->mMaterials[materialIndex];
    if (material->GetTextureCount(type) > 0) {
        aiString texPath;
        material->GetTexture(type, 0, &texPath);
        return std::string(texPath.C_Str());
    }

    return "";
}