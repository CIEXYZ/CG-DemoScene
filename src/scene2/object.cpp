#include "object.h"
#include <ppgso/ppgso.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>
#include <assimp/scene.h>

#include "scene.h"
#include "ModelLoader.h"

#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>


Object::Object(const std::string& modelPath) {
    shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    mesh = std::make_shared<ppgso::Mesh_Assimp>(modelPath);

    auto allTextures = ModelLoader::loadAllTextures(mesh.get());

    for (const auto& pair : allTextures) {
        materialTextures[pair.first] = pair.second.diffuse;
        normalMaps[pair.first] = pair.second.normal;
        if (pair.second.specular) {
            specularMaps[pair.first] = pair.second.specular;
        }
    }

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
                materialTextures[i] = texture;

                std::string normalPath = ModelLoader::findNormalMapByName(texturePath, mesh->getBasePath());
                if (!normalPath.empty()) {
                    normalMaps[i] = ModelLoader::loadTextureFile(normalPath);
                } else {
                    normalMaps[i] = ModelLoader::createDefaultNormalMap();
                }
            }
        }
    } else {
        // If custom texture fails, use default textures from mesh
        auto allTextures = ModelLoader::loadAllTextures(mesh.get());
        for (const auto& pair : allTextures) {
            materialTextures[pair.first] = pair.second.diffuse;
            normalMaps[pair.first] = pair.second.normal;
        }
    }
}

void Object::setMesh(const std::string& meshPath) {
    mesh = std::make_unique<ppgso::Mesh>(meshPath);
}

void Object::setTransparentMeshes(const std::vector<std::string>& meshNames) {
    transparentMeshNames.clear();
    for (const auto& name : meshNames) {
        transparentMeshNames.insert(name);
    }
}

bool Object::isMeshTransparent(const std::string& meshName) const {
    // Check for prefix match
    for (const auto& transparentName : transparentMeshNames) {
        if (meshName.find(transparentName) == 0) {
            return true;
        }
    }

    return false;
}

void Object::setShader(const std::string& vertShader, const std::string& fragShader) {
    shader = std::make_unique<ppgso::Shader>(vertShader, fragShader);
}

void Object::setNormalMap(unsigned int materialIndex, const std::string& normalMapPath) {
    std::cout << "Loading normal map for material " << materialIndex << ": " << normalMapPath << std::endl;

    auto normalTexture = ModelLoader::loadTextureFile(normalMapPath);
    if (normalTexture) {
        normalMaps[materialIndex] = normalTexture;
        std::cout << "Normal map loaded successfully" << std::endl;
    } else {
        std::cerr << "Failed to load normal map: " << normalMapPath << std::endl;
        normalMaps[materialIndex] = ModelLoader::createDefaultNormalMap();
    }
}

void Object::update(Scene &scene, float dt) {
    modelMatrix = glm::mat4(1.0f);

    if (hasCollision) {
        boundingBox.update(position, scale * boxSize, boxOffset);
    }

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

    if (isParticle) {
        if (usesGravity) {
            glm::vec3 gravity = glm::vec3(0.f, -9.81f, 0.f) * mass;
            applyForces(gravity);
        }

        acceleration = forces / mass;
        velocity += acceleration * dt;
        velocity *= drag;

        oldPosition = position;
        position = position + (velocity) * dt;
        clearForce();

        glm::vec3 angularAcceleration = torque / momentOfInertia;
        angularVelocity += angularAcceleration * dt;
        angularVelocity *= 0.99f; //drag

        float maxAngularSpeed = 10.0f;  // radians per second
        if (glm::length(angularVelocity) > maxAngularSpeed) {
            angularVelocity = glm::normalize(angularVelocity) * maxAngularSpeed;
        }

        rotation += angularVelocity * dt;
        rotation.x = fmod(rotation.x, 6.28318f);
        rotation.y = fmod(rotation.y, 6.28318f);
        rotation.z = fmod(rotation.z, 6.28318f);

        clearTorque();

        if (hasCollision) {
            boundingBox.update(position, scale * boxSize, boxOffset);
        }

        isGrounded = false;

        for (auto &obj : scene.objects) {
            bool shouldResponse = obj.get() != this && obj->hasCollision && collidesWithOther(*obj);

            if (shouldResponse) {
                bool landingOnTop = velocity.y < 0.0f && position.y > obj->boundingBox.max.y - 0.1f;

                if (landingOnTop) {
                    stopOnYAxis(obj);
                    isGrounded = true;
                    break;
                }else {
                    if (velocity.x != .0f) {
                        stopOnXAxis(obj);
                    }
                    if (velocity.z != .0f) {
                        stopOnZAxis(obj);
                    }
                    break;
                }
            }
        }
    }
    if (hasCollision) {
        boundingBox.update(position, scale * boxSize, boxOffset);
    }

}

void Object::stopOnYAxis(std::shared_ptr<Object> &obj) {
    velocity.y = 0.0f;
    position.y = obj->boundingBox.max.y + (boundingBox.getSize().y * .5f);
    boundingBox.update(position, scale * boxSize, boxOffset);
}

void Object::stopOnXAxis(std::shared_ptr<Object> &obj) {
    if (oldPosition.x < obj->position.x) {
        position.x = obj->boundingBox.min.x - (boundingBox.getSize().x * .5f);
    }else {
        position.x = obj->boundingBox.max.x + (boundingBox.getSize().x * .5f);
    }
    velocity.x = 0.0f;
    boundingBox.update(position, scale * boxSize, boxOffset);
}

void Object::stopOnZAxis(std::shared_ptr<Object> &obj) {
    if (oldPosition.z < obj->position.z) {
        position.z = obj->boundingBox.min.z - (boundingBox.getSize().z * .5f);
    }else {
        position.z = obj->boundingBox.max.z + (boundingBox.getSize().z * .5f);
    }
    velocity.z = .0f;
    boundingBox.update(position, scale * boxSize, boxOffset);
}

// Help find aiMesh by name and calculate its center
glm::vec3 findMeshCenterByName(const aiScene* scene, const std::string& meshName,
                                const glm::mat4& nodeTransform, const aiNode* node = nullptr) {
    if (!scene) return glm::vec3(0.0f);

    // Start from root if node not provided
    if (!node) {
        node = scene->mRootNode;
    }

    // Build current transform
    aiMatrix4x4 aiTrans = node->mTransformation;
    glm::mat4 currentTransform = glm::mat4(
        aiTrans.a1, aiTrans.b1, aiTrans.c1, aiTrans.d1,
        aiTrans.a2, aiTrans.b2, aiTrans.c2, aiTrans.d2,
        aiTrans.a3, aiTrans.b3, aiTrans.c3, aiTrans.d3,
        aiTrans.a4, aiTrans.b4, aiTrans.c4, aiTrans.d4
    );
    glm::mat4 globalTransform = nodeTransform * currentTransform;

    // Check meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* aiMeshData = scene->mMeshes[node->mMeshes[i]];
        if (aiMeshData && std::string(aiMeshData->mName.C_Str()) == meshName) {
            if (aiMeshData->mNumVertices == 0) {
                return glm::vec3(0.0f);
            }

            glm::vec3 center(0.0f);
            for (unsigned int v = 0; v < aiMeshData->mNumVertices; ++v) {
                glm::vec4 vertex = glm::vec4(
                    aiMeshData->mVertices[v].x,
                    aiMeshData->mVertices[v].y,
                    aiMeshData->mVertices[v].z,
                    1.0f
                );
                glm::vec4 transformedVertex = globalTransform * vertex;
                center += glm::vec3(transformedVertex);
            }
            center /= static_cast<float>(aiMeshData->mNumVertices);
            return center;
        }
    }

    // Recursively search children
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        glm::vec3 result = findMeshCenterByName(scene, meshName, globalTransform, node->mChildren[i]);
        if (result != glm::vec3(0.0f)) {
            return result;
        }
    }

    return glm::vec3(0.0f);
}

void Object::render(Scene &scene) {
    if (!shouldRender){return;}
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
    shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));
    shader->setUniform("useNormalMap", useNormalMapping);

    // shader->setUniform("testLightPos", scene.pointLights[0].position);
    // shader->setUniform("testLightColor", glm::vec3(1.0f, 0.0f, 0.0f));

    // glm::vec3 lightDirection = glm::normalize(glm::vec3(5.0f, 5.0f, 5.0f)); //TODO: Global lightning (potom pouzivat lightDirection zo scene.h)
    // shader->setUniform("LightDirection", lightDirection);

    size_t meshCount = mesh->getMeshCount();
    const aiScene* aiSceneData = mesh->getScene();

    // First pass: render opaque meshes
    shader->setUniform("DisableLighting", false);
    shader->setUniform("Transparency", 1.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // forceTransparent is used for dust particles
    for (size_t i = 0; i < meshCount; ++i) {
        bool isTransparent = isMeshTransparent(mesh->getMeshName(i));

        // Skip transparent meshes in first pass
        if (isTransparent || billboarding || forceTransparent) continue;

        // Set diffuse texture
        auto it = materialTextures.find(mesh->getMaterialIndex(i));
        if (it != materialTextures.end()) {
            shader->setUniform("Texture", *(it->second));
        }

        // Set normal map if using normal mapping
        if (useNormalMapping) {
            auto normalIt = normalMaps.find(mesh->getMaterialIndex(i));
            if (normalIt != normalMaps.end()) {
                shader->setUniform("NormalMap", *(normalIt->second));
            }
        }

        mesh->renderSingleMesh(i);
    }

    // Second pass: render transparent meshes
    if (transparentMeshNames.empty() && !billboarding && !forceTransparent) {
        return;
    }

    // Collect transparent mesh indices with their distances from camera
    struct TransparentMeshInfo {
        size_t index;
        float distance;
    };
    std::vector<TransparentMeshInfo> transparentMeshes;

    glm::vec3 cameraPos = scene.camera->position;

    for (size_t i = 0; i < meshCount; ++i) {
        bool isTransparent = isMeshTransparent(mesh->getMeshName(i));

        if (isTransparent || billboarding  || forceTransparent) {
            // Find the mesh center by searching through the scene hierarchy
            std::string meshName = mesh->getMeshName(i);
            glm::vec3 meshLocalCenter = findMeshCenterByName(aiSceneData, meshName, glm::mat4(1.0f));

            // Transform to world space
            glm::vec3 meshWorldCenter = glm::vec3(modelMatrix * glm::vec4(meshLocalCenter, 1.0f));
            float distance = glm::length(cameraPos - meshWorldCenter);

            transparentMeshes.push_back({i, distance});
        }
    }

    // Sort transparent meshes by distance
    std::sort(transparentMeshes.begin(), transparentMeshes.end(), [](const TransparentMeshInfo& a, const TransparentMeshInfo& b)
        {return a.distance > b.distance;});

    // Set transparent state
    shader->setUniform("Transparency", transparency);
    shader->setUniform("DisableLighting", true);// we are disabling lighting because it makes clouds black when looking at them form the back
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);//needed to render particles from the back

    // Render transparent meshes in sorted order
    for (const auto& meshInfo : transparentMeshes) {
        // Set diffuse texture
        auto it = materialTextures.find(mesh->getMaterialIndex(meshInfo.index));
        if (it != materialTextures.end()) {
            shader->setUniform("Texture", *(it->second));
        }

        // Set normal map if using normal mapping
        if (useNormalMapping) {
            auto normalIt = normalMaps.find(mesh->getMaterialIndex(meshInfo.index));
            if (normalIt != normalMaps.end()) {
                shader->setUniform("NormalMap", *(normalIt->second));
            }
        }

        mesh->renderSingleMesh(meshInfo.index);
    }

    // Reset to default state
    shader->setUniform("Transparency", 1.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

}

void Object::generateModelMatrix() {
    modelMatrix =
            glm::translate(glm::mat4(1.0f), position)
            * glm::orientate4(rotation)
            * glm::scale(glm::mat4(1.0f), scale);
}

bool Object::collidesWithOther(const Object &otherObject) const {
    if (hasCollision || otherObject.hasCollision) {
        return boundingBox.collidesWithOther(otherObject.boundingBox);
    }
}

void Object::applyForces(const glm::vec3 &force) {
    if (isParticle) {
        forces += force;
    }
}

void Object::clearForce() {
    forces = glm::vec3(0.0f);
}
