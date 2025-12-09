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

    // Setup instanced attributes for all meshes
    size_t meshCount = animMesh->getMeshCount();
    for (size_t i = 0; i < meshCount; ++i) {
        animMesh->setupInstancedAttributes(i, instanceMatrixVBO, instanceTextureVBO);
    }

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
    shader->setUniform("useInstancing", false);

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
    shader->setUniform("usePointLight", scene.usePointLight);
    if (scene.usePointLight) {
        shader->setUniform("pointRedLight.position", scene.redLight.position);
        shader->setUniform("pointRedLight.ambient", scene.redLight.ambient);
        shader->setUniform("pointRedLight.diffuse", scene.redLight.diffuse);
        shader->setUniform("pointRedLight.specular", scene.redLight.specular);
        shader->setUniform("pointRedLight.constant", scene.redLight.constant);
        shader->setUniform("pointRedLight.linear", scene.redLight.linear);
        shader->setUniform("pointRedLight.quadratic", scene.redLight.quadratic);

        shader->setUniform("pointGreenLight.position", scene.greenLight.position);
        shader->setUniform("pointGreenLight.ambient", scene.greenLight.ambient);
        shader->setUniform("pointGreenLight.diffuse", scene.greenLight.diffuse);
        shader->setUniform("pointGreenLight.specular", scene.greenLight.specular);
        shader->setUniform("pointGreenLight.constant", scene.greenLight.constant);
        shader->setUniform("pointGreenLight.linear", scene.greenLight.linear);
        shader->setUniform("pointGreenLight.quadratic", scene.greenLight.quadratic);

        shader->setUniform("pointBlueLight.position", scene.blueLight.position);
        shader->setUniform("pointBlueLight.ambient", scene.blueLight.ambient);
        shader->setUniform("pointBlueLight.diffuse", scene.blueLight.diffuse);
        shader->setUniform("pointBlueLight.specular", scene.blueLight.specular);
        shader->setUniform("pointBlueLight.constant", scene.blueLight.constant);
        shader->setUniform("pointBlueLight.linear", scene.blueLight.linear);
        shader->setUniform("pointBlueLight.quadratic", scene.blueLight.quadratic);

        shader->setUniform("pointCeilingLight.position", scene.ceilingLight.position);
        shader->setUniform("pointCeilingLight.ambient", scene.ceilingLight.ambient);
        shader->setUniform("pointCeilingLight.diffuse", scene.ceilingLight.diffuse);
        shader->setUniform("pointCeilingLight.specular", scene.ceilingLight.specular);
        shader->setUniform("pointCeilingLight.constant", scene.ceilingLight.constant);
        shader->setUniform("pointCeilingLight.linear", scene.ceilingLight.linear);
        shader->setUniform("pointCeilingLight.quadratic", scene.ceilingLight.quadratic);
    }

    //SPOTLIGHTS
    shader->setUniform("useSpotLight", scene.useSpotLight);
    if (scene.useSpotLight) {
        shader->setUniform("spotDJLights.position", scene.djSpot.position);
        shader->setUniform("spotDJLights.direction", scene.djSpot.direction);
        shader->setUniform("spotDJLights.ambient", scene.djSpot.ambient);
        shader->setUniform("spotDJLights.diffuse", scene.djSpot.diffuse);
        shader->setUniform("spotDJLights.specular", scene.djSpot.specular);
        shader->setUniform("spotDJLights.cutOff", scene.djSpot.cutOff);
        shader->setUniform("spotDJLights.outerCutOff", scene.djSpot.outerCutOff);
        shader->setUniform("spotDJLights.constant", scene.djSpot.constant);
        shader->setUniform("spotDJLights.linear", scene.djSpot.linear);
        shader->setUniform("spotDJLights.quadratic", scene.djSpot.quadratic);

        shader->setUniform("spotDanceLights.position", scene.danceSpot.position);
        shader->setUniform("spotDanceLights.direction", scene.danceSpot.direction);
        shader->setUniform("spotDanceLights.ambient", scene.danceSpot.ambient);
        shader->setUniform("spotDanceLights.diffuse", scene.danceSpot.diffuse);
        shader->setUniform("spotDanceLights.specular", scene.danceSpot.specular);
        shader->setUniform("spotDanceLights.cutOff", scene.danceSpot.cutOff);
        shader->setUniform("spotDanceLights.outerCutOff", scene.danceSpot.outerCutOff);
        shader->setUniform("spotDanceLights.constant", scene.danceSpot.constant);
        shader->setUniform("spotDanceLights.linear", scene.danceSpot.linear);
        shader->setUniform("spotDanceLights.quadratic", scene.danceSpot.quadratic);
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
    shader->setUniform("useInstancing", true);

    if (hasAnimation && animator) {
        shader->setUniform("hasAnimation", true);
        auto boneMatrices = animator->getFinalBoneMatrices();

        for (size_t i = 0; i < boneMatrices.size(); ++i) {
            std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
            shader->setUniform(uniformName, boneMatrices[i]);
        }
    } else {
        shader->setUniform("hasAnimation", false);
    }

    shader->setUniform("DisableLighting", false);
    shader->setUniform("Transparency", 1.0f);

    // Setup texture array
    std::vector<ppgso::Texture*> textureArray;
    for (const auto& [idx, tex] : materialTextures) {
        textureArray.push_back(tex.get());
    }

    shader->setUniform("TextureArraySize", (int)textureArray.size());

    // Bind textures
    for (size_t i = 0; i < textureArray.size() && i < 16; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textureArray[i]->getTexture());
        std::string uniformName = "TextureArray[" + std::to_string(i) + "]";
        shader->setUniform(uniformName, (int)i);
    }

    size_t meshCount = animMesh->getMeshCount();

    // Render all meshes
    for (size_t i = 0; i < meshCount; ++i) {
        animMesh->renderSingleMeshInstanced(i, instances.size());
    }

    glActiveTexture(GL_TEXTURE0);
}