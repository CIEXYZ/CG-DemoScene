//
// Created by majer on 12/6/2025.
//

#ifndef PPGSO_DUSTEMITTER_H
#define PPGSO_DUSTEMITTER_H


#pragma once

#include "object.h"
#include "DustParticle.h"
#include <vector>
#include <memory>

class DustEmitter : public Object {
public:
    DustEmitter(std::shared_ptr<ppgso::Mesh_Assimp> particleMesh,
                const std::string& particleTexture);

    void update(Scene &scene, float dt) override;
    void render(Scene &scene) override;

    std::shared_ptr<ppgso::Mesh_Assimp> particleMesh;
    std::shared_ptr<ppgso::Texture> particleTexture;
    std::vector<std::unique_ptr<DustParticle>> particles;
    float spawnRate = 22.0f;  // Particles per second 12
    float lastSpawnTime = 0.0f;
    bool isActive = true;
    int maxParticles = 120; //change to 120

    // Spawn area (particles spawn randomly within this box) this main hall
    glm::vec3 spawnAreaMin = glm::vec3(-2.0f, 0.0f, -5.f);
    glm::vec3 spawnAreaMax = glm::vec3(2.0f, 1.f, 5.0f);
};


#endif //PPGSO_DUSTEMITTER_H