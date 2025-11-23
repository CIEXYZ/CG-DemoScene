//
// Created by majer on 11/18/2025.
//
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

//AABB na fyziku pouzit (dym, postavy, pohar, odraz objektu od telesa(nejaky ohnostroj od plochy)
//ten dym nemusi byt komplikovany nemusi mat kolizie moze prechdazat cez objekty
//bullet by zabral mozno moc vela casu na natiahnutie do projektu
//Spatial partitioning moc komplikovane

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    float alpha;
    float size;
};