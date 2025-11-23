#include "object.h"
#include <ppgso/ppgso.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "scene.h"
#include "ModelLoader.h"

#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>

Object::Object(const std::string& modelPath) {
    shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    mesh = std::make_shared<ppgso::Mesh_Assimp>(modelPath);
    materialTextures = ModelLoader::loadTextures(mesh.get());
    ownsMesh = true;
}

Object::Object(std::shared_ptr<ppgso::Mesh_Assimp> sharedMesh, const std::string& texturePath) {
    shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    mesh = sharedMesh;
    ownsMesh = false;

    // Load custom texture for all materials
    auto texture = ModelLoader::loadTextureFile(texturePath);
    if (texture) {
        // Apply this texture to all material slots
        const aiScene* scene = mesh->getScene();
        if (scene) {
            for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
                materialTextures[i] = ModelLoader::loadTextureFile(texturePath);
            }
        }
    } else {
        // If custom texture fails, use default textures from mesh
        materialTextures = ModelLoader::loadTextures(mesh.get());
    }
}

void Object::setMesh(const std::string& meshPath) {
    mesh = std::make_unique<ppgso::Mesh>(meshPath);
}

void Object::setShader(const std::string& vertShader, const std::string& fragShader) {
    shader = std::make_unique<ppgso::Shader>(vertShader, fragShader);
}

void Object::update(Scene &scene, float dt) {
    modelMatrix = glm::mat4(1.0f);

    if (billboarding) {
        glm::mat4 viewMatrix = scene.camera->viewMatrix;

        modelMatrix = glm::translate(modelMatrix, position);

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                modelMatrix[i][j] = viewMatrix[j][i];
            }
        }

        modelMatrix = glm::scale(modelMatrix, scale);

    } else {
        //Transformations
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, scale);
    }
}

void Object::render(Scene &scene) {
    shader->use();

    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    shader->setUniform("ModelMatrix", modelMatrix);

    glm::vec3 lightDirection = glm::normalize(glm::vec3(5.0f, 5.0f, 5.0f)); //TODO: Global lightning (potom pouzivat lightDirection zo scene.h)
    shader->setUniform("LightDirection", lightDirection);

    if (billboarding) {
        shader->setUniform("DisableLighting", true);
        shader->setUniform("Transparency", transparency);

        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }



    //Render all meshes with own textures
    mesh->renderWithMaterials([this](unsigned int materialIndex) {
      auto it = materialTextures.find(materialIndex);
      if (it != materialTextures.end()) {
        shader->setUniform("Texture", *(it->second));
      }
    });

    if (billboarding) {
        shader->setUniform("DisableLighting", false);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

void Object::generateModelMatrix() {
    modelMatrix =
            glm::translate(glm::mat4(1.0f), position)
            * glm::orientate4(rotation)
            * glm::scale(glm::mat4(1.0f), scale);
}
