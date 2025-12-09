#pragma once

#include "object.h"
#include <ppgso/ppgso.h>
#include <GL/glew.h>

class DanceFloorObject : public Object {
public:
    DanceFloorObject();
    virtual ~DanceFloorObject();

    void update(Scene &scene, float dt) override;
    void render(Scene &scene) override;

    static constexpr int GRID_SIZE = 12;
    std::vector<std::vector<glm::vec3>> cubeColors;

private:
    void init();
    void initGrid();

    GLuint VAO, VBO, EBO, TBO, NBO;
    int indexCount;
    std::shared_ptr<ppgso::Texture> texture;

    struct CubeInstance {
        glm::vec3 position;
        glm::vec3 scale;
    };
    std::vector<std::vector<CubeInstance>> gridCubes;
    CubeInstance platform;
};