//
// Created by majer on 12/5/2025.
//

#include "DustParticle.h"
#include <glm/gtc/random.hpp>
#include <shaders/diffuse_frag_glsl.h>
#include <shaders/diffuse_vert_glsl.h>

DustParticle::DustParticle(std::shared_ptr<ppgso::Mesh_Assimp> mesh, std::shared_ptr<ppgso::Texture> texture)
    : Object(mesh, "") {

    billboarding = false;
    isStatic = false;
    hasCollision = true;
    usesGravity = true;

    mass = glm::linearRand(0.02f, 0.05f);
    drag = 0.98f;

    this->mesh = mesh;
    this->shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    this->ownsMesh = false;
    this->materialTextures[0] = texture;

    age = 0.0f;
    life = glm::linearRand(5.0f, 10.0f);
    fadeStart = 0.7f;
    initialScale = glm::linearRand(0.3f, 0.6f);  // Visible size
    scale = glm::vec3(initialScale);

    velocity = glm::vec3(
        glm::linearRand(-1.5f, 1.5f),
        glm::linearRand(1.0f, 3.0f),   // Launch upward
        glm::linearRand(-1.5f, 1.5f)
    );

    momentOfInertia = mass * initialScale * initialScale * 0.4f;  // I = m × r² × 0.4 (sphere approx)

    angularVelocity = glm::vec3(
        glm::linearRand(-3.0f, 3.0f),
        glm::linearRand(-3.0f, 3.0f),
        glm::linearRand(-3.0f, 3.0f)
    );
}

void DustParticle::update(Scene &scene, float dt) {
    age += dt;


    // Optional FORCE 3: Random turbulence
    if (glm::linearRand(0.0f, 1.0f) < 0.05f) {  // 5% chance
        glm::vec3 turbulence = glm::vec3(
            glm::linearRand(-1.0f, 1.0f),
            glm::linearRand(-0.5f, 0.5f),
            glm::linearRand(-1.0f, 1.0f)
        ) * mass * 0.3f;
        applyForces(turbulence);
    }


    glm::vec3 velocityTorque = glm::cross(velocity, glm::vec3(0.0f, 1.0f, 0.0f)) * 0.1f;
    applyTorque(velocityTorque);

    if (glm::linearRand(0.0f, 1.0f) < 0.1f) {  // 10% chance
        glm::vec3 randomTorque = glm::vec3(
            glm::linearRand(-0.5f, 0.5f),
            glm::linearRand(-0.5f, 0.5f),
            glm::linearRand(-0.5f, 0.5f)
        );
        applyTorque(randomTorque);
    }


    // Fade in/out
    float lifeRatio = age / life;
    if (lifeRatio < 0.2f) {
        transparency = lifeRatio / 0.2f;
    } else if (lifeRatio > fadeStart) {
        transparency = 1.0f - ((lifeRatio - fadeStart) / (1.0f - fadeStart));
    } else {
        transparency = 1.0f;
    }
    transparency *= 0.7f;

    // ============================================
    // ✅ Call Object::update
    // This handles:
    // - Applying gravity force (FORCE 1)
    // - Applying wind force (FORCE 2)
    // - Applying drag force
    // - Linear motion: a=F/m, v+=a×dt, p+=v×dt
    // - Angular motion: α=τ/I, ω+=α×dt, θ+=ω×dt
    // - Collision detection and response
    // ============================================
    Object::update(scene, dt);

    // Debug output
    static int frameCount = 0;
    if (frameCount++ % 120 == 0) {  // Every 2 seconds
        std::cout << "\n=== Particle Physics Demo ===" << std::endl;
        std::cout << "FORCES APPLIED:" << std::endl;
        std::cout << "  Gravity: (0, -9.81, 0) × " << mass << " = (0, " << -9.81f * mass << ", 0)" << std::endl;
        std::cout << "  Drag: -velocity × (1 - " << drag << ")" << std::endl;
        std::cout << "\nLINEAR MOTION:" << std::endl;
        std::cout << "  Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        std::cout << "  Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
        std::cout << "  Acceleration: (" << acceleration.x << ", " << acceleration.y << ", " << acceleration.z << ")" << std::endl;
        std::cout << "\nANGULAR MOTION (RIGID BODY):" << std::endl;
        std::cout << "  Rotation: (" << rotation.x << ", " << rotation.y << ", " << rotation.z << ")" << std::endl;
        std::cout << "  Angular Velocity: (" << angularVelocity.x << ", " << angularVelocity.y << ", " << angularVelocity.z << ")" << std::endl;
        std::cout << "  Torque: (" << torque.x << ", " << torque.y << ", " << torque.z << ")" << std::endl;
        std::cout << "  Moment of Inertia: " << momentOfInertia << std::endl;
    }
}