#include <glm/glm.hpp>
#include <sstream>

#include "Mesh_Assimp.h"

std::string getParentPath(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? "" : path.substr(0, pos);
}

ppgso::Mesh_Assimp::Mesh_Assimp(const std::string &obj_file) {
#ifdef DEBBUG_MODE
    std::cout << "Using ASSIMP Loader!" << std::endl;
#endif

    // Store base path for later use
    basePath = getParentPath(obj_file);

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
    std::cout << "Root node children: " << scene->mRootNode->mNumChildren << std::endl;

    processNode(scene->mRootNode, scene, glm::mat4(1.0f));

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

void ppgso::Mesh_Assimp::processNode(aiNode *node, const aiScene *pScene, const glm::mat4& parentTransform) {
    aiMatrix4x4 aiTrans = node->mTransformation;
    glm::mat4 nodeTransform = glm::mat4(
        aiTrans.a1, aiTrans.b1, aiTrans.c1, aiTrans.d1,
        aiTrans.a2, aiTrans.b2, aiTrans.c2, aiTrans.d2,
        aiTrans.a3, aiTrans.b3, aiTrans.c3, aiTrans.d3,
        aiTrans.a4, aiTrans.b4, aiTrans.c4, aiTrans.d4
    );

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *mesh = pScene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, globalTransform);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], pScene, globalTransform);
    }
}

void ppgso::Mesh_Assimp::processMesh(aiMesh *mesh, const glm::mat4& transform) {
    gl_buffer buffer;

    // Store material index and mesh name for this mesh
    buffer.materialIndex = mesh->mMaterialIndex;
    buffer.meshIndex = 0;
    buffer.meshName = mesh->mName.C_Str();

    std::cout << "Processing mesh: '" << buffer.meshName << "' with material index: " << buffer.materialIndex << std::endl;

    // Process vertices
    if (mesh->HasPositions()) {
        // Generate a vertex array object
        glGenVertexArrays(1, &buffer.vao);
        glBindVertexArray(buffer.vao);

        std::vector<glm::vec3> transformedVertices;
        transformedVertices.reserve(mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            glm::vec4 vertex = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
            glm::vec4 transformedVertex = transform * vertex;
            transformedVertices.push_back(glm::vec3(transformedVertex));
        }

        // Extract vertex positions from aiMesh and upload to GPU
        glGenBuffers(1, &buffer.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
        glBufferData(GL_ARRAY_BUFFER, transformedVertices.size() * sizeof(glm::vec3), transformedVertices.data(), GL_STATIC_DRAW);
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
        std::vector<glm::vec3> transformedNormals;
        transformedNormals.reserve(mesh->mNumVertices);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            glm::vec3 normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            glm::vec3 transformedNormal = glm::normalize(normalMatrix * normal);
            transformedNormals.push_back(transformedNormal);
        }

        glGenBuffers(1, &buffer.nbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.nbo);
        glBufferData(GL_ARRAY_BUFFER, transformedNormals.size() * sizeof(glm::vec3), transformedNormals.data(), GL_STATIC_DRAW);

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

void ppgso::Mesh_Assimp::renderSingleMesh(size_t bufferIndex) {
    if (bufferIndex < buffers.size()) {
        auto &buffer = buffers[bufferIndex];
        glBindVertexArray(buffer.vao);
        glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);
    }
}

void ppgso::Mesh_Assimp::renderWithMaterials(std::function<void(unsigned int, size_t)> setMaterialCallback) {
    for (size_t i = 0; i < buffers.size(); ++i) {
        auto &buffer = buffers[i];

        // Set material for this mesh part, passing buffer index
        setMaterialCallback(buffer.materialIndex, i);

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