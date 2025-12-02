//
// Created by majer on 11/22/2025.
//
#include "Emitter.h"
#include "ModelLoader.h"
#include "scene.h"

Emitter::Emitter(std::shared_ptr<ppgso::Mesh_Assimp> smokeMesh, const std::string &smokeTexture)
    : Object(smokeMesh, smokeTexture){
    this->smokeTexture = ModelLoader::loadTextureFile(smokeTexture);
    this->smokeMesh = smokeMesh;

    spawnRate = 2.f;
    lastSpawnTime = 0.f;
    isActive = true;
    spawnPosition = glm::vec3(0.0f, 0.0f, 0.0f);

}

void Emitter::update(Scene &scene, float dt) {
    if (isActive) {
        lastSpawnTime += dt;

        float spawnInterval = 1.f / spawnRate;

        if (lastSpawnTime > spawnInterval) {
            auto cloud = std::make_unique<SmokeCloud>(smokeMesh, smokeTexture);
            cloud->position = position + spawnPosition;
            clouds.push_back(std::move(cloud));
            lastSpawnTime = 0.f;
        }
    }

    for (auto it = clouds.begin(); it != clouds.end();) {
        (*it)->update(scene, dt);

        if ((*it)->age >= (*it)->life) {
            it = clouds.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Emitter::render(Scene &scene) {
    if (clouds.empty()) return;

    glm::vec3 camPosition = scene.camera->position;

    std::sort(clouds.begin(), clouds.end(),
        [&camPosition](const std::unique_ptr<SmokeCloud>&a, const std::unique_ptr<SmokeCloud>& b) {
            float distanceA = glm::distance(camPosition, a->position);
            float distanceB = glm::distance(camPosition, b->position);
            return distanceA > distanceB;
        });

    for (auto &cloud : clouds) {
        cloud->render(scene);
    }
}
