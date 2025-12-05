//
// Created by majer on 11/18/2025.
//
//
// #ifndef PPGSO_SMOKECLOUD_H
// #define PPGSO_SMOKECLOUD_H
//
// #endif //PPGSO_SMOKECLOUD_H
#pragma once
#include "object.h"
#include <random>

class SmokeCloud : public Object {
public:
    SmokeCloud(std::shared_ptr<ppgso::Mesh_Assimp> mesh, std::shared_ptr<ppgso::Texture> sharedTexture);
    void update(Scene &scene, float dt) override;

    float life;
    float age;
    float fadeStart;
    float initialScale;
};