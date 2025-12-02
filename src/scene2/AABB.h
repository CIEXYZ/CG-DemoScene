//
// Created by majer on 11/18/2025.
//
//
// #ifndef PPGSO_PARTICLE_H
// #define PPGSO_PARTICLE_H
//
// #endif //PPGSO_PARTICLE_H
#pragma once
#include <glm/glm.hpp>

class AABB {
    public:
    glm::vec3 min;
    glm::vec3 max;

    AABB();
    AABB(const glm::vec3& min, const glm::vec3& max);

    bool collidesWithOther(const AABB &otherObject) const;
    void update(const glm::vec3 &position, const glm::vec3 &scale, const glm::vec3 &offset = glm::vec3(0.f));

    glm::vec3 getSize() const;
    glm::vec3 getCenter() const;

    bool containsPoint(const glm::vec3& point) const;
};
