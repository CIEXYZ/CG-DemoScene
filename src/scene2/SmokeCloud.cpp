//
// Created by majer on 11/18/2025.
//

//sprite dymu na priesvitnom stvorcovom podklade ktory musi byt paralelny so scenou?(aby s kazdeho uhlu pohladu bol paralelny so scenou)
    //toto je v chate aj tie particles

#include "SmokeCloud.h"
#include <glm/gtc/random.hpp>
#include <shaders/diffuse_frag_glsl.h>
#include <shaders/diffuse_vert_glsl.h>

SmokeCloud::SmokeCloud(std::shared_ptr<ppgso::Mesh_Assimp> mesh, const std::string& texturePath) : Object(mesh, texturePath) {
    billboarding = true;

    age = 0.0f;
    life = glm::linearRand(3.0f, 6.0f);
    fadeStart = 0.6f;
    initialScale = glm::linearRand(.5f, 1.0f);
    scale = glm::vec3(initialScale);

    velocity = glm::vec3(
        glm::linearRand(-.3f, .3f), //x
        glm::linearRand(.5f, 1.0f), //y
        glm::linearRand(-.3f, .3f) //z
    );
}
// constructor for cashed textures
SmokeCloud::SmokeCloud(std::shared_ptr<ppgso::Mesh_Assimp> mesh, std::shared_ptr<ppgso::Texture> sharedTexture) : Object(mesh, "") {
    billboarding = true;

    this->mesh = mesh;
    this->shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    this->ownsMesh = false;
    this->materialTextures[0] = sharedTexture;

    age = 0.0f;
    life = glm::linearRand(3.0f, 6.0f);
    fadeStart = 0.6f;
    initialScale = glm::linearRand(.5f, 1.0f);
    scale = glm::vec3(initialScale);

    velocity = glm::vec3(
        glm::linearRand(-.3f, .3f), //x
        glm::linearRand(.5f, 1.0f), //y
        glm::linearRand(-.3f, .3f) //z
    );
}

void SmokeCloud::update(Scene &scene, float dt) {
    age += dt;
    position += velocity * dt;
    float growth = 1.f + (age/life)*2.f;
    float lifeRation = age/life;

    if (lifeRation > fadeStart) {
        transparency = 1.0f - ((lifeRation - fadeStart)/ (1.0f - fadeStart));
    } else {
        transparency = 1.f;
    }

    Object::update(scene, dt);
}