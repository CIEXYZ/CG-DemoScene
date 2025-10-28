#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <functional>

#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "texture.h"

// Edit by: Samuel Zaprazny
// Adding assimp library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace ppgso {

    class Mesh_Assimp {
        struct gl_buffer {
        public:
            GLuint vao, vbo, tbo, nbo, ibo = 0;
            GLsizei size = 0;
            unsigned int materialIndex = 0; // Added: material index for this mesh
        };

        std::vector<gl_buffer> buffers;

        // Keep importer alive so scene pointer remains valid
        Assimp::Importer importer;
        const aiScene * scene;

        // Store base path for texture loading
        std::string basePath;

        // Loaded materials
        std::vector<glm::vec3> ambient;
        std::vector<glm::vec3> diffuse;
        std::vector<glm::vec3> specular;

        struct MaterialTextures {
            std::unique_ptr<ppgso::Texture> diffuse;
            std::unique_ptr<ppgso::Texture> normal;
            std::unique_ptr<ppgso::Texture> specular;
        };
        std::vector<MaterialTextures> textures;

    public:

        /*!
         * Load 3D geometry from a na Wavefront .obj file.
         *
         * The shader program passed to the object will be bound to the geometry as follows:
         * vec3 Position - Vertex position, position 0
         * vec2 TexCoord - Texture coordinate, position 1
         * vec3 Normal - Normal vector, position 2
         *
         * @param obj - File path to the obj file to load.
         */
        Mesh_Assimp(const std::string &obj);

        ~Mesh_Assimp();

        void processNode(aiNode *node, const aiScene *pScene);

        void processMesh(aiMesh *mesh);

        const std::vector<MaterialTextures>& getTextures() const { return textures; }

        // Get material index for a specific buffer
        unsigned int getMaterialIndex(size_t bufferIndex) const {
            if (bufferIndex < buffers.size()) {
                return buffers[bufferIndex].materialIndex;
            }
            return 0;
        }

        // Get number of meshes/buffers
        size_t getMeshCount() const { return buffers.size(); }

        // Get texture path for a material
        std::string getTexturePath(unsigned int materialIndex, aiTextureType type) const;

        // Get the scene (for accessing materials)
        const aiScene* getScene() const { return scene; }

        // Get base path
        const std::string& getBasePath() const { return basePath; }

        /*!
         * Render the geometry associated with the mesh using glDrawElements.
         */
        void render();

        /*!
         * Render with callback for setting material per mesh part
         */
        void renderWithMaterials(std::function<void(unsigned int)> setMaterialCallback);
    };
}