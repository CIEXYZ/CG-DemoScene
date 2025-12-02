#include "AnimatedMesh.h"
#include <iostream>

std::string getParentPathHelper(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? "" : path.substr(0, pos);
}

AnimatedMesh::AnimatedMesh(const std::string& modelPath) {
    basePath = getParentPathHelper(modelPath);

    scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        std::cerr << importer.GetErrorString() << std::endl;
        return;
    }
    
    std::cout << "AnimatedMesh loaded: " << modelPath << std::endl;
    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;
    
    // Extract bone info first
    boneCount = 0;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int b = 0; b < mesh->mNumBones; b++) {
            std::string boneName = mesh->mBones[b]->mName.C_Str();

            if (boneMapping.find(boneName) == boneMapping.end()) {
                boneMapping[boneName] = boneCount;
                boneCount++;
            }
        }
    }
    std::cout << "Total bones in model: " << boneCount << std::endl;
    
    // Process all meshes
    processNode(scene->mRootNode, scene);
}

AnimatedMesh::~AnimatedMesh() {
    for (auto& buffer : buffers) {
        glDeleteBuffers(1, &buffer.ibo);
        glDeleteBuffers(1, &buffer.nbo);
        glDeleteBuffers(1, &buffer.tbo);
        glDeleteBuffers(1, &buffer.vbo);
        glDeleteBuffers(1, &buffer.boneIdsBuffer);
        glDeleteBuffers(1, &buffer.boneWeightsBuffer);
        glDeleteVertexArrays(1, &buffer.vao);
    }
}

void AnimatedMesh::processNode(aiNode* node, const aiScene* pScene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = pScene->mMeshes[node->mMeshes[i]];
        processMesh(mesh);
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], pScene);
    }
}

void AnimatedMesh::extractBoneWeights(aiMesh* mesh, std::vector<VertexBoneData>& boneData) {
    boneData.resize(mesh->mNumVertices);
    
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiBone* bone = mesh->mBones[i];
        std::string boneName = bone->mName.C_Str();
        
        int boneID = -1;
        if (boneMapping.find(boneName) != boneMapping.end()) {
            boneID = boneMapping[boneName];
        } else {
            std::cerr << "Warning: Bone not in mapping: " << boneName << std::endl;
            continue;
        }
        
        for (unsigned int j = 0; j < bone->mNumWeights; j++) {
            unsigned int vertexID = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            boneData[vertexID].addBoneData(boneID, weight);
        }
    }
}

void AnimatedMesh::processMesh(aiMesh* mesh) {
    gl_buffer buffer;
    buffer.materialIndex = mesh->mMaterialIndex;
    buffer.meshName = mesh->mName.C_Str();
    
    std::cout << "Processing animated mesh: '" << buffer.meshName  << "' with " << mesh->mNumBones << " bones" << std::endl;

    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    if (mesh->HasPositions()) {
        std::vector<glm::vec3> vertices;
        vertices.reserve(mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            vertices.push_back(glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            ));
        }
        
        glGenBuffers(1, &buffer.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (mesh->HasTextureCoords(0)) {
        std::vector<glm::vec2> texCoords;
        texCoords.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            texCoords.push_back(glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            ));
        }
        
        glGenBuffers(1, &buffer.tbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.tbo);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (mesh->HasNormals()) {
        std::vector<glm::vec3> normals;
        normals.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            normals.push_back(glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            ));
        }
        
        glGenBuffers(1, &buffer.nbo);
        glBindBuffer(GL_ARRAY_BUFFER, buffer.nbo);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    
    // Extract and upload bone data
    std::vector<VertexBoneData> boneData;
    extractBoneWeights(mesh, boneData);
    
    // Upload bone ID
    std::vector<glm::ivec4> boneIDs;
    boneIDs.reserve(boneData.size());
    for (const auto& vbd : boneData) {
        boneIDs.push_back(glm::ivec4(
            vbd.boneIDs[0], vbd.boneIDs[1],
            vbd.boneIDs[2], vbd.boneIDs[3]
        ));
    }
    
    glGenBuffers(1, &buffer.boneIdsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.boneIdsBuffer);
    glBufferData(GL_ARRAY_BUFFER, boneIDs.size() * sizeof(glm::ivec4), boneIDs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(glm::ivec4), (void*)0);
    
    // Upload bone weights
    std::vector<glm::vec4> boneWeights;
    boneWeights.reserve(boneData.size());
    for (const auto& vbd : boneData) {
        boneWeights.push_back(glm::vec4(
            vbd.weights[0], vbd.weights[1],
            vbd.weights[2], vbd.weights[3]
        ));
    }
    
    glGenBuffers(1, &buffer.boneWeightsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.boneWeightsBuffer);
    glBufferData(GL_ARRAY_BUFFER, boneWeights.size() * sizeof(glm::vec4), boneWeights.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    
    // Upload indices
    if (mesh->HasFaces()) {
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }
        
        glGenBuffers(1, &buffer.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        buffer.size = static_cast<GLsizei>(indices.size());
    }
    
    glBindVertexArray(0);
    buffers.push_back(buffer);
    
    std::cout << "Uploaded " << boneData.size() << " vertices with bone data" << std::endl;
}

void AnimatedMesh::render() {
    for (auto& buffer : buffers) {
        glBindVertexArray(buffer.vao);
        glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);
    }
}

void AnimatedMesh::renderSingleMesh(size_t bufferIndex) {
    if (bufferIndex < buffers.size()) {
        auto& buffer = buffers[bufferIndex];
        glBindVertexArray(buffer.vao);
        glDrawElements(GL_TRIANGLES, buffer.size, GL_UNSIGNED_INT, nullptr);
    }
}

std::string AnimatedMesh::getMeshName(size_t bufferIndex) const {
    if (bufferIndex < buffers.size()) {
        return buffers[bufferIndex].meshName;
    }
    return "";
}

unsigned int AnimatedMesh::getMaterialIndex(size_t bufferIndex) const {
    if (bufferIndex < buffers.size()) {
        return buffers[bufferIndex].materialIndex;
    }
    return 0;
}