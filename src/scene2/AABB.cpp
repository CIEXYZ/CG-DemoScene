//
// Created by majer on 11/18/2025.
//
#include <glm/glm.hpp>
#include "AABB.h"

#include <iostream>
#include <ostream>

//AABB na fyziku pouzit (postavy, pohar, odraz objektu od telesa(nejaky ohnostroj od plochy)
//bullet by zabral mozno moc vela casu na natiahnutie do projektu
//Spatial partitioning moc komplikovane

AABB::AABB () : min(0.0f), max(0.0f) {}

AABB::AABB(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) {}

bool AABB::collidesWithOther(const AABB &otherObject) const {
    bool xCollision = min.x <= otherObject.max.x && max.x >= otherObject.min.x;
    bool yCollision = min.y <= otherObject.max.y && max.y >= otherObject.min.y;
    bool zCollision = min.z <= otherObject.max.z && max.z >= otherObject.min.z;

    std::cout << "This AABB: min(" << min.x << ", " << min.y << ", " << min.z
             << ") max(" << max.x << ", " << max.y << ", " << max.z << ")" << std::endl;
    std::cout << "Other AABB: min(" << otherObject.min.x << ", " << otherObject.min.y << ", " << otherObject.min.z
              << ") max(" << otherObject.max.x << ", " << otherObject.max.y << ", " << otherObject.max.z << ")" << std::endl;
    std::cout << "Collisions: X=" << xCollision << " Y=" << yCollision << " Z=" << zCollision << std::endl;

    return xCollision && yCollision && zCollision;
}

void AABB::update(const glm::vec3 &position, const glm::vec3 &scale, const glm::vec3 &offset) {
    glm::vec3 halfSize = glm::vec3(.5f) * scale;

    min = position + offset - halfSize;
    max = position + offset + halfSize;
}

glm::vec3 AABB::getSize() const {
    return max - min;
}

glm::vec3 AABB::getCenter() const {
    return (min + max) * 0.5f;
}

bool AABB::containsPoint(const glm::vec3& point) const {
    return (point.x >= min.x && point.x <= max.x) &&
       (point.y >= min.y && point.y <= max.y) &&
       (point.z >= min.z && point.z <= max.z);
}