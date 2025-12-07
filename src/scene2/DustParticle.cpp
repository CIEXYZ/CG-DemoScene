//
// Created by majer on 12/5/2025.
//

#include "DustParticle.h"
#include <glm/gtc/random.hpp>
#include <shaders/diffuse_frag_glsl.h>
#include <shaders/diffuse_vert_glsl.h>

DustParticle::DustParticle(std::shared_ptr<ppgso::Mesh_Assimp> mesh, std::shared_ptr<ppgso::Texture> texture)
    : Object(mesh, "") {
    forceTransparent = true;
    billboarding = false;
    isParticle = true;
    hasCollision = true;
    usesGravity = true;
    boxSize = glm::vec3(1.f); // this is somehow working correctly even though it should be 2. in case of plane obj due to its size


    mass = glm::linearRand(0.002f, 0.005f); //this should be okay for a dust weight
    drag = 0.6f; //high drag 6

    this->mesh = mesh;
    this->shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    this->ownsMesh = false;
    this->materialTextures[0] = texture;

    scale = glm::vec3(initialScale);

    velocity = glm::vec3(
        glm::linearRand(-1.5f, 3.f),
        glm::linearRand(1.0f, 3.0f),
        glm::linearRand(-1.5f, 3.f)
    );

    momentOfInertia = mass * initialScale * initialScale * 0.4f;  // I = m × r² × 0.4 (sphere approx)

    angularVelocity = glm::vec3(
        glm::linearRand(-1.0f, 1.0f),
        glm::linearRand(-1.0f, 1.0f),
        glm::linearRand(-1.0f, 1.0f)
    );
}

void DustParticle::update(Scene &scene, float dt) {
    age += dt;


    //random turbulence
    if (glm::linearRand(0.0f, 1.0f) < .30f) {  // 5% chance
        glm::vec3 turbulence = glm::vec3(
            glm::linearRand(-25.0f, 25.0f),
            glm::linearRand(-20.5f, 20.5f),
            glm::linearRand(-25.0f, 25.0f)
        ) * mass * 0.3f;
        applyForces(turbulence);
    }


    glm::vec3 velocityTorque = glm::cross(velocity, glm::vec3(0.0f, 1.0f, 0.0f)) * 0.015f;
    applyTorque(velocityTorque);

    //works like a fucking charm
    if (age < 0.5f) {
        transparency = (age / 0.5f) * 0.7f;
    }
    else if (isGrounded) {
        transparency *=0.95f;
    } else {
        transparency = 0.7f;  // Full visibility
    }
    Object::update(scene, dt);

}