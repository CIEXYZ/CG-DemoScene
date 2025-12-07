//
// Created by majer on 12/5/2025.
//

#ifndef PPGSO_DUSTPARTICLE_H
#define PPGSO_DUSTPARTICLE_H

#include "object.h"
#include <glm/glm.hpp>
#include <memory>

class DustParticle : public Object {
public:
    DustParticle(std::shared_ptr<ppgso::Mesh_Assimp> mesh, std::shared_ptr<ppgso::Texture> texture);
    void update(Scene &scene, float dt) override;

    float age = 0.0f;
    float life = 12.0f;
    float initialScale = glm::linearRand(0.01f, 0.03f);
    float fadeoutDur = 2.f;
    float timeSinceGrounded = 0.0f;

};


#endif //PPGSO_DUSTPARTICLE_H