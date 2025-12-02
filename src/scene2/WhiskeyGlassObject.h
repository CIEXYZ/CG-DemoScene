#pragma once

#include "object.h"
#include <ppgso/ppgso.h>
#include <GL/glew.h>

class WhiskeyGlassObject : public Object {
public:
    WhiskeyGlassObject(const std::string& modelPath);
    ~WhiskeyGlassObject();

    void update(Scene &scene, float dt) override;
    void render(Scene &scene) override;

private:
    void init();
    void initLiquidCube();
    void initIceCubes();
    glm::vec3 evaluateBezierPatch(float u, float v);

    // Bezier liquid surface
    GLuint liquidVAO, liquidVBO, liquidEBO;
    int liquidIndexCount;
    std::vector<glm::vec3> liquidVertices;
    std::vector<glm::vec3> liquidNormals;
    std::vector<unsigned int> liquidIndices;

    // Bézier control points (4x4 grid)
    glm::vec3 controlPoints[4][4];
    float liquidSize = 0.24f;

    // Ice cubes
    struct IceCube {
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
    };
    std::vector<IceCube> iceCubes;
    GLuint cubeVAO, cubeVBO, cubeEBO;
    int cubeIndexCount;

    std::unique_ptr<ppgso::Shader> liquidShader;
    std::unique_ptr<ppgso::Shader> cubeShader;

    float time;
};