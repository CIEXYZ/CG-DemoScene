#pragma once

#include "Mesh_Assimp.h"
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

#define MAX_BONE_INFLUENCE 4

struct VertexBoneData {
    int boneIDs[MAX_BONE_INFLUENCE];
    float weights[MAX_BONE_INFLUENCE];

    VertexBoneData() {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            boneIDs[i] = -1;
            weights[i] = 0.0f;
        }
    }

    void addBoneData(int boneID, float weight) {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (weights[i] == 0.0f) {
                boneIDs[i] = boneID;
                weights[i] = weight;
                return;
            }
        }
    }
};

// Animated mesh that handles bone data
class AnimatedMesh {
private:
    struct gl_buffer {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint tbo = 0;
        GLuint nbo = 0;
        GLuint ibo = 0;

        GLuint boneIdsBuffer = 0;
        GLuint boneWeightsBuffer = 0;
        GLsizei size = 0;

        unsigned int materialIndex = 0;
        std::string meshName;
    };

    std::vector<gl_buffer> buffers;
    const aiScene* scene = nullptr;
    Assimp::Importer importer;
    std::string basePath;
    
    std::map<std::string, int> boneMapping;
    int boneCount = 0;

    void processNode(aiNode* node, const aiScene* pScene);
    void processMesh(aiMesh* mesh);
    void extractBoneWeights(aiMesh* mesh, std::vector<VertexBoneData>& boneData);

public:
    AnimatedMesh(const std::string& modelPath);
    ~AnimatedMesh();

    void render();
    void renderSingleMesh(size_t bufferIndex);
    
    size_t getMeshCount() const { return buffers.size(); }
    std::string getMeshName(size_t bufferIndex) const;
    unsigned int getMaterialIndex(size_t bufferIndex) const;
    
    const aiScene* getScene() const { return scene; }
    const std::string& getBasePath() const { return basePath; }
    const std::map<std::string, int>& getBoneMapping() const { return boneMapping; }
};