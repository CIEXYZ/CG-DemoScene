//
// Created by majer on 12/8/2025.
//

#ifndef PPGSO_LIGHTING_H
#define PPGSO_LIGHTING_H

#pragma once
#include <glm/glm.hpp>
#include <vector>


struct Material {
    glm::vec3 ambient = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
    float shininess = 32.0f;
};

struct DirectionalLight {
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct PointLight {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    // fallof parameters
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    // fallof based on distance
    void setDistance(float distance) {
        if (distance <= 7) {
            constant = 1.0f; linear = 0.7f; quadratic = 1.8f;
        }
        else if (distance <= 13) {
            constant = 1.0f; linear = 0.35f; quadratic = 0.44f;
        }
        else if (distance <= 20) {
            constant = 1.0f; linear = 0.22f; quadratic = 0.20f;
        }
        else if (distance <= 32) {
            constant = 1.0f; linear = 0.14f; quadratic = 0.07f;
        }
        else if (distance <= 50) {
            constant = 1.0f; linear = 0.09f; quadratic = 0.032f;
        }
        else if (distance <= 65) {
            constant = 1.0f; linear = 0.07f; quadratic = 0.017f;
        }
        else if (distance <= 100) {
            constant = 1.0f; linear = 0.045f; quadratic = 0.0075f;
        }
        else if (distance <= 160) {
            constant = 1.0f; linear = 0.027f; quadratic = 0.0028f;
        }
        else if (distance <= 200) {
            constant = 1.0f; linear = 0.022f; quadratic = 0.0019f;
        }
        else if (distance <= 325) {
            constant = 1.0f; linear = 0.014f; quadratic = 0.0007f;
        }
        else {
            constant = 1.0f; linear = 0.007f; quadratic = 0.0002f;
        }
    }
};

struct SpotLight {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(15.0f));

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    //Set spotlight angle
    void setAngle(float innerDegrees, float outerDegrees) {
        cutOff = glm::cos(glm::radians(innerDegrees));
        outerCutOff = glm::cos(glm::radians(outerDegrees));
    }
};


#endif //PPGSO_LIGHTING_H