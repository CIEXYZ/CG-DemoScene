//
// Created by majer on 11/18/2025.
//
#include <glm/glm.hpp>
#include "AABB.h"

//AABB na fyziku pouzit (postavy, pohar, odraz objektu od telesa(nejaky ohnostroj od plochy)
//bullet by zabral mozno moc vela casu na natiahnutie do projektu
//Spatial partitioning moc komplikovane

AABB::AABB () : min(0.0f), max(0.0f) {}

AABB::AABB(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) {}

bool AABB::collidesWithOther(const AABB &otherObject) const {
    return (min.x <= otherObject.max.x && max.x >= otherObject.min.x) &&
       (min.y <= otherObject.max.y && max.y >= otherObject.min.y) &&
       (min.z <= otherObject.max.z && max.z >= otherObject.min.z);
}

void AABB::update(const glm::vec3 &position, const glm::vec3 &scale, const glm::vec3 &offset) {
    glm::vec3 halfSize = glm::vec3(.5f) * scale;

    min = position + halfSize - offset;
    max = position + halfSize + offset;
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