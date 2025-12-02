//
// Created by majer on 11/22/2025.
//
//
// #ifndef PPGSO_EMITTER_H
// #define PPGSO_EMITTER_H
//
// #endif //PPGSO_EMITTER_H
#pragma once
#include "object.h"
#include "SmokeCloud.h"
#include <vector>
#include <memory>

class Emitter : public Object {

public:
    Emitter(std::shared_ptr<ppgso::Mesh_Assimp> smokeMesh, const std::string& smokeTexture);

    void update(Scene &scene, float dt) override;
    void render(Scene &scene) override;

    float spawnRate;
    glm::vec3 spawnPosition;
    bool isActive;
    float lastSpawnTime;


    std::shared_ptr<ppgso::Mesh_Assimp> smokeMesh;
    std::shared_ptr<ppgso::Texture> smokeTexture;
    std::vector<std::unique_ptr<SmokeCloud>> clouds;
};