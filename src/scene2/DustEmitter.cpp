//
// Created by majer on 12/6/2025.
//

#include "DustEmitter.h"
#include "ModelLoader.h"
#include "scene.h"
#include <glm/gtc/random.hpp>

DustEmitter::DustEmitter(std::shared_ptr<ppgso::Mesh_Assimp> particleMesh, const std::string& particleTexture)
    : Object(particleMesh, particleTexture) {

    this->particleMesh = particleMesh;
    this->particleTexture = ModelLoader::loadTextureFile(particleTexture);

    hasCollision = false;
    usesGravity = false;
    billboarding = false;
}

void DustEmitter::update(Scene &scene, float dt) {
    if (isActive && particles.size() < maxParticles) {
        lastSpawnTime += dt;
        float spawnInterval = 1.0f / spawnRate;

        while (lastSpawnTime >= spawnInterval && particles.size() < maxParticles) {
            //Create particle with cached texture
            auto particle = std::make_unique<DustParticle>(particleMesh, particleTexture);

            // Random position within spawn area
            particle->position = position + glm::vec3(
                glm::linearRand(spawnAreaMin.x, spawnAreaMax.x),
                glm::linearRand(spawnAreaMin.y, spawnAreaMax.y),
                glm::linearRand(spawnAreaMin.z, spawnAreaMax.z)
            );

            particles.push_back(std::move(particle));
            lastSpawnTime -= spawnInterval;
        }
    }

    // Update existing particles
    for (auto it = particles.begin(); it != particles.end();) {
        (*it)->update(scene, dt);

        // Remove dead particles
        if (((*it)->isGrounded && (*it)->transparency < 0.01f) || ((*it)->life < (*it)->age) ) {
            it = particles.erase(it);
        } else {
        ++it;
        }
    }
}

void DustEmitter::render(Scene &scene) {
    if (particles.empty()) return;

    // Render all particles
    for (auto &particle : particles) {
        particle->render(scene);
    }
}