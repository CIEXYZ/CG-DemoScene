#include "PersonObject.h"
#include "ModelLoader.h"
#include <iostream>

// Use the same diffuse fragment shader
#include <shaders/animated_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>

PersonObject::PersonObject(const std::string& modelPath) {
    // Create animated mesh
    animMesh = std::make_unique<AnimatedMesh>(modelPath);

    const aiScene* scene = animMesh->getScene();
    if (scene) {
        mesh = std::make_shared<ppgso::Mesh_Assimp>(modelPath);
        materialTextures = ModelLoader::loadTextures(mesh.get());
    }

    // Set animated shader
    shader = std::make_unique<ppgso::Shader>(animated_vert_glsl, diffuse_frag_glsl);
    hasAnimation = false;

    std::cout << "PersonObject created (no animation)" << std::endl;
}

PersonObject::PersonObject(const std::string& modelPath, const std::string& animationPath) {
    // Create animated mesh
    animMesh = std::make_unique<AnimatedMesh>(modelPath);

    // Load textures
    const aiScene* scene = animMesh->getScene();
    if (scene) {
        mesh = std::make_shared<ppgso::Mesh_Assimp>(modelPath);
        materialTextures = ModelLoader::loadTextures(mesh.get());
    }

    // Set animated shader
    shader = std::make_unique<ppgso::Shader>(animated_vert_glsl, diffuse_frag_glsl);

    // Load animation
    loadAnimation(animationPath);

    std::cout << "PersonObject created with animation" << std::endl;
}

void PersonObject::loadAnimation(const std::string& animationPath) {
    if (!animMesh) {
        std::cerr << "Cannot load animation: animated mesh not created!" << std::endl;
        return;
    }

    const aiScene* scene = animMesh->getScene();
    if (!scene) {
        std::cerr << "Cannot load animation: scene not available!" << std::endl;
        return;
    }

    auto animation = std::make_unique<Animation>(animationPath, scene);
    animations.push_back(std::move(animation));

    if (!animator && !animations.empty()) {
        animator = std::make_unique<Animator>(animations[0].get());
        hasAnimation = true;
        std::cout << "Animation loaded successfully" << std::endl;
    }
}

void PersonObject::playAnimation(int animationIndex) {
    if (animationIndex >= 0 && animationIndex < animations.size() && animator) {
        animator->playAnimation(animations[animationIndex].get());
        std::cout << "Playing animation " << animationIndex << std::endl;
    }
}

void PersonObject::setupInstanceBuffers(size_t maxInst) {
    if (instanceBuffersSetup && maxInst == maxInstances) {
        return;
    }

    cleanupInstanceBuffers();

    maxInstances = maxInst;

    glGenBuffers(1, &instanceMatrixVBO);
    glGenBuffers(1, &instanceTextureVBO);

    instanceBuffersSetup = true;
}

void PersonObject::cleanupInstanceBuffers() {
    if (instanceMatrixVBO != 0) {
        glDeleteBuffers(1, &instanceMatrixVBO);
        instanceMatrixVBO = 0;
    }
    if (instanceTextureVBO != 0) {
        glDeleteBuffers(1, &instanceTextureVBO);
        instanceTextureVBO = 0;
    }
    instanceBuffersSetup = false;
}

void PersonObject::update(Scene &scene, float dt) {
    // Update animation
    if (hasAnimation && animator) {
        animator->updateAnimation(dt);
    }

    // Update model matrix
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, scale);
}

void PersonObject::render(Scene &scene) {
    if (!animMesh) return;

    shader->use();

    //BASIC
    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    shader->setUniform("ModelMatrix", modelMatrix);

    //MATERIAL PROPERTIES
    shader->setUniform("material.ambient", scene.defaultMaterial.ambient);
    shader->setUniform("material.diffuse", scene.defaultMaterial.diffuse);
    shader->setUniform("material.specular", scene.defaultMaterial.specular);
    shader->setUniform("material.shininess", scene.defaultMaterial.shininess);

    //CAMERA POSITION
    shader->setUniform("viewPos", scene.camera->position);

    //DIRECTIONAL
    shader->setUniform("useDirLight", scene.useDirLight);
    if (scene.useDirLight) {
        shader->setUniform("dirLight.direction", scene.directionalLight.direction);
        shader->setUniform("dirLight.ambient", scene.directionalLight.ambient);
        shader->setUniform("dirLight.diffuse", scene.directionalLight.diffuse);
        shader->setUniform("dirLight.specular", scene.directionalLight.specular);
    }
    //POINT
    shader->setUniform("numPointLights", (int)scene.pointLights.size());
    for (size_t i = 0; i < scene.pointLights.size() && i < 10; i++) {
        std::string base = "pointLights[" + std::to_string(i) + "]";
        shader->setUniform(base + ".position", scene.pointLights[i].position);
        shader->setUniform(base + ".ambient", scene.pointLights[i].ambient);
        shader->setUniform(base + ".diffuse", scene.pointLights[i].diffuse);
        shader->setUniform(base + ".specular", scene.pointLights[i].specular);
        shader->setUniform(base + ".constant", scene.pointLights[i].constant);
        shader->setUniform(base + ".linear", scene.pointLights[i].linear);
        shader->setUniform(base + ".quadratic", scene.pointLights[i].quadratic);
    }

    //SPOTLIGHTS
    shader->setUniform("numSpotLights", (int)scene.spotLights.size());
    for (size_t i = 0; i < scene.spotLights.size() && i < 5; i++) {
        std::string base = "spotLights[" + std::to_string(i) + "]";
        shader->setUniform(base + ".position", scene.spotLights[i].position);
        shader->setUniform(base + ".direction", scene.spotLights[i].direction);
        shader->setUniform(base + ".ambient", scene.spotLights[i].ambient);
        shader->setUniform(base + ".diffuse", scene.spotLights[i].diffuse);
        shader->setUniform(base + ".specular", scene.spotLights[i].specular);
        shader->setUniform(base + ".cutOff", scene.spotLights[i].cutOff);
        shader->setUniform(base + ".outerCutOff", scene.spotLights[i].outerCutOff);
        shader->setUniform(base + ".constant", scene.spotLights[i].constant);
        shader->setUniform(base + ".linear", scene.spotLights[i].linear);
        shader->setUniform(base + ".quadratic", scene.spotLights[i].quadratic);
    }

     //LIGHTING OPTIONS
    shader->setUniform("useBlinnPhong", scene.useBlinnPhong);

    // shader->setUniform("testLightPos", scene.pointLights[0].position);
    // shader->setUniform("testLightColor", glm::vec3(1.0f, 0.0f, 0.0f));

    if (hasAnimation && animator) {
        shader->setUniform("hasAnimation", true);
        auto matrices = animator->getFinalBoneMatrices();

        for (size_t i = 0; i < matrices.size(); ++i) {
            std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
            shader->setUniform(uniformName, matrices[i]);
        }
    } else {
        shader->setUniform("hasAnimation", false);
    }

    size_t meshCount = animMesh->getMeshCount();
    shader->setUniform("DisableLighting", false);
    shader->setUniform("Transparency", 1.0f);

    for (size_t i = 0; i < meshCount; ++i) {
        unsigned int matIndex = animMesh->getMaterialIndex(i);
        auto it = materialTextures.find(matIndex);
        if (it != materialTextures.end()) {
            shader->setUniform("Texture", *(it->second));
        }

        animMesh->renderSingleMesh(i);
    }
}

void PersonObject::renderInstanced(Scene &scene, const std::vector<InstanceData>& instances) {
    if (!animMesh || instances.empty()) return;

    setupInstanceBuffers(instances.size());

    // Prepare instance data arrays
    std::vector<glm::mat4> matrices;
    std::vector<float> textureIndices;
    matrices.reserve(instances.size());
    textureIndices.reserve(instances.size());

    for (const auto& instance : instances) {
        matrices.push_back(instance.modelMatrix);
        textureIndices.push_back(static_cast<float>(instance.textureIndex));
    }

    // Upload instance matrices to GPU
    glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixVBO);
    glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(glm::mat4), matrices.data(), GL_DYNAMIC_DRAW);

    // Upload texture indices to GPU
    glBindBuffer(GL_ARRAY_BUFFER, instanceTextureVBO);
    glBufferData(GL_ARRAY_BUFFER, textureIndices.size() * sizeof(float), textureIndices.data(), GL_DYNAMIC_DRAW);

    // Use regular shader
    shader->use();

    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);

    if (hasAnimation && animator) {
        shader->setUniform("hasAnimation", true);
        auto matrices = animator->getFinalBoneMatrices();

        for (size_t i = 0; i < matrices.size(); ++i) {
            std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
            shader->setUniform(uniformName, matrices[i]);
        }
    } else {
        shader->setUniform("hasAnimation", false);
    }

    shader->setUniform("DisableLighting", false);
    shader->setUniform("Transparency", 1.0f);

    size_t meshCount = animMesh->getMeshCount();

    // Render each instance
    for (const auto& instance : instances) {
        shader->setUniform("ModelMatrix", instance.modelMatrix);

        for (size_t i = 0; i < meshCount; ++i) {
            unsigned int matIndex = animMesh->getMaterialIndex(i);

            // Use the texture index from instance data
            auto it = materialTextures.find(instance.textureIndex);
            if (it != materialTextures.end()) {
                shader->setUniform("Texture", *(it->second));
            } else {
                // Fallback to material texture
                it = materialTextures.find(matIndex);
                if (it != materialTextures.end()) {
                    shader->setUniform("Texture", *(it->second));
                }
            }

            animMesh->renderSingleMesh(i);
        }
    }
}