#include "WhiskeyGlassObject.h"
#include "scene.h"
#include "ModelLoader.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>

WhiskeyGlassObject::WhiskeyGlassObject(const std::string& modelPath) : Object(modelPath) {
    init();
}

WhiskeyGlassObject::~WhiskeyGlassObject() {
    glDeleteVertexArrays(1, &liquidVAO);
    glDeleteBuffers(1, &liquidVBO);
    glDeleteBuffers(1, &liquidEBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);
}

void WhiskeyGlassObject::init() {
    liquidShader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);
    cubeShader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);

    // Control points position
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            controlPoints[i][j] = glm::vec3(
                -(liquidSize / 4.3f) + j * (liquidSize / 6.4f),
                liquidSize / 5.0f,
                -(liquidSize / 4.3f) + i * (liquidSize / 6.4f)
            );
        }
    }
    initLiquidCube();
    initIceCubes();
}

glm::vec3 WhiskeyGlassObject::evaluateBezierPatch(float u, float v) {
    // Evaluate 4 bezier curves (for each row)
    glm::vec3 uCurves[4];
    for (int i = 0; i < 4; i++) {
        glm::vec3 u01 = glm::mix(controlPoints[i][0], controlPoints[i][1], u);
        glm::vec3 u12 = glm::mix(controlPoints[i][1], controlPoints[i][2], u);
        glm::vec3 u23 = glm::mix(controlPoints[i][2], controlPoints[i][3], u);

        glm::vec3 u02 = glm::mix(u01, u12, u);
        glm::vec3 u13 = glm::mix(u12, u23, u);

        uCurves[i] = glm::mix(u02, u13, u);
    }

    // Evaluate final curve
    glm::vec3 v01 = glm::mix(uCurves[0], uCurves[1], v);
    glm::vec3 v12 = glm::mix(uCurves[1], uCurves[2], v);
    glm::vec3 v23 = glm::mix(uCurves[2], uCurves[3], v);

    glm::vec3 v02 = glm::mix(v01, v12, v);
    glm::vec3 v13 = glm::mix(v12, v23, v);

    return glm::mix(v02, v13, v);
}

void WhiskeyGlassObject::initLiquidCube() {
    liquidVertices.clear();
    liquidNormals.clear();
    liquidIndices.clear();

    const int resolution = 20;
    float halfSize = liquidSize / 5.0f;

    int bezierStartIndex = 0;

    // Generate bezier top surface
    for (int i = 0; i <= resolution; i++) {
        for (int j = 0; j <= resolution; j++) {
            float u = (float)j / resolution;
            float v = (float)i / resolution;

            glm::vec3 point = evaluateBezierPatch(u, v);
            liquidVertices.push_back(point);
            liquidNormals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            int topLeft = bezierStartIndex + i * (resolution + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = bezierStartIndex + (i + 1) * (resolution + 1) + j;
            int bottomRight = bottomLeft + 1;

            // First triangle
            liquidIndices.push_back(topLeft);
            liquidIndices.push_back(bottomLeft);
            liquidIndices.push_back(topRight);

            // Second triangle
            liquidIndices.push_back(topRight);
            liquidIndices.push_back(bottomLeft);
            liquidIndices.push_back(bottomRight);
        }
    }

    //Generate 5 side faces with bezier edge
    int cubeStartIndex = liquidVertices.size();

    // Bottom
    glm::vec3 bottomFrontLeft = glm::vec3(-halfSize, -halfSize,  halfSize);
    glm::vec3 bottomFrontRight = glm::vec3( halfSize, -halfSize,  halfSize);
    glm::vec3 bottomBackLeft = glm::vec3(-halfSize, -halfSize, -halfSize);
    glm::vec3 bottomBackRight = glm::vec3( halfSize, -halfSize, -halfSize);

    liquidVertices.push_back(bottomBackLeft);
    liquidVertices.push_back(bottomBackRight);
    liquidVertices.push_back(bottomFrontRight);
    liquidVertices.push_back(bottomFrontLeft);

    liquidNormals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
    liquidNormals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
    liquidNormals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
    liquidNormals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));

    liquidIndices.push_back(cubeStartIndex + 0);
    liquidIndices.push_back(cubeStartIndex + 1);
    liquidIndices.push_back(cubeStartIndex + 2);
    liquidIndices.push_back(cubeStartIndex + 0);
    liquidIndices.push_back(cubeStartIndex + 2);
    liquidIndices.push_back(cubeStartIndex + 3);

    // Front
    int frontFaceStart = liquidVertices.size();

    for (int u = 0; u <= resolution; u++) {
        // Bottom vertex
        float t = (float)u / resolution;
        glm::vec3 bottomVertex = glm::mix(bottomFrontLeft, bottomFrontRight, t);
        liquidVertices.push_back(bottomVertex);
        liquidNormals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

        // Top vertex with bezier
        int bezierIndex = resolution * (resolution + 1) + u;
        liquidVertices.push_back(liquidVertices[bezierIndex]);
        liquidNormals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    }

    for (int u = 0; u < resolution; u++) {
        int bl = frontFaceStart + u * 2;
        int br = frontFaceStart + (u + 1) * 2;
        int tl = bl + 1;
        int tr = br + 1;

        liquidIndices.push_back(bl);
        liquidIndices.push_back(br);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(bl);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(tl);
    }

    // Back
    int backFaceStart = liquidVertices.size();

    for (int u = 0; u <= resolution; u++) {
        // Bottom vertex
        float t = (float)u / resolution;
        glm::vec3 bottomVertex = glm::mix(bottomBackRight, bottomBackLeft, t);
        liquidVertices.push_back(bottomVertex);
        liquidNormals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));

        // Top vertex with bezier
        int bezierIndex = resolution - u;
        liquidVertices.push_back(liquidVertices[bezierIndex]);
        liquidNormals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    }

    for (int u = 0; u < resolution; u++) {
        int bl = backFaceStart + u * 2;
        int br = backFaceStart + (u + 1) * 2;
        int tl = bl + 1;
        int tr = br + 1;

        liquidIndices.push_back(bl);
        liquidIndices.push_back(br);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(bl);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(tl);
    }

    // Left
    int leftFaceStart = liquidVertices.size();

    for (int v = 0; v <= resolution; v++) {
        // Bottom vertex
        float t = (float)v / resolution;
        glm::vec3 bottomVertex = glm::mix(bottomBackLeft, bottomFrontLeft, t);
        liquidVertices.push_back(bottomVertex);
        liquidNormals.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));

        // Top vertex with bezier
        int bezierIndex = v * (resolution + 1);
        liquidVertices.push_back(liquidVertices[bezierIndex]);
        liquidNormals.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
    }

    for (int v = 0; v < resolution; v++) {
        int bl = leftFaceStart + v * 2;
        int br = leftFaceStart + (v + 1) * 2;
        int tl = bl + 1;
        int tr = br + 1;

        liquidIndices.push_back(bl);
        liquidIndices.push_back(br);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(bl);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(tl);
    }

    // Right
    int rightFaceStart = liquidVertices.size();

    for (int v = 0; v <= resolution; v++) {
        // Bottom vertex
        float t = (float)v / resolution;
        glm::vec3 bottomVertex = glm::mix(bottomFrontRight, bottomBackRight, t);
        liquidVertices.push_back(bottomVertex);
        liquidNormals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

        // Top vertex with bezier
        int bezierIndex = (resolution - v) * (resolution + 1) + resolution;
        liquidVertices.push_back(liquidVertices[bezierIndex]);
        liquidNormals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    }

    for (int v = 0; v < resolution; v++) {
        int bl = rightFaceStart + v * 2;
        int br = rightFaceStart + (v + 1) * 2;
        int tl = bl + 1;
        int tr = br + 1;

        liquidIndices.push_back(bl);
        liquidIndices.push_back(br);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(bl);
        liquidIndices.push_back(tr);
        liquidIndices.push_back(tl);
    }

    liquidIndexCount = liquidIndices.size();

    glGenVertexArrays(1, &liquidVAO);
    glGenBuffers(1, &liquidVBO);
    glGenBuffers(1, &liquidEBO);

    glBindVertexArray(liquidVAO);

    glBindBuffer(GL_ARRAY_BUFFER, liquidVBO);
    glBufferData(GL_ARRAY_BUFFER, liquidVertices.size() * sizeof(glm::vec3) + liquidNormals.size() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, liquidVertices.size() * sizeof(glm::vec3), liquidVertices.data());
    glBufferSubData(GL_ARRAY_BUFFER, liquidVertices.size() * sizeof(glm::vec3), liquidNormals.size() * sizeof(glm::vec3), liquidNormals.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, liquidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, liquidIndices.size() * sizeof(unsigned int), liquidIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(liquidVertices.size() * sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void WhiskeyGlassObject::initIceCubes() {
    iceCubes.clear();

    IceCube cube1;
    cube1.position = glm::vec3(0.02f, -0.02f, 0.01f);
    cube1.scale = glm::vec3(0.03f, 0.03f, 0.03f);
    cube1.rotation = glm::vec3(0.3f, 0.5f, 0.2f);
    iceCubes.push_back(cube1);

    IceCube cube2;
    cube2.position = glm::vec3(-0.02f, 0.01f, -0.01f);
    cube2.scale = glm::vec3(0.03f, 0.03f, 0.03f);
    cube2.rotation = glm::vec3(0.7f, 0.3f, 0.9f);
    iceCubes.push_back(cube2);

    IceCube cube3;
    cube3.position = glm::vec3(-0.01f, 0.03f, 0.02f);
    cube3.scale = glm::vec3(0.03f, 0.03f, 0.03f);
    cube3.rotation = glm::vec3(0.4f, 0.8f, 0.1f);
    iceCubes.push_back(cube3);

    // Create cube geometry
    std::vector<glm::vec3> cubeVertices = {
        // Front face
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},

        // Back face
        {-0.5f, -0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},

        // Top face
        {-0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        { 0.5f,  0.5f, -0.5f},

        // Bottom face
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f,  0.5f},
        {-0.5f, -0.5f,  0.5f},

        // Right face
        { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        { 0.5f,  0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},

        // Left face
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f, -0.5f}
    };

    std::vector<glm::vec3> cubeNormals = {
        // Front face
        {0.0f,  0.0f,  1.0f},
        {0.0f,  0.0f,  1.0f},
        {0.0f,  0.0f,  1.0f},
        {0.0f,  0.0f,  1.0f},

        // Back face
        {0.0f,  0.0f, -1.0f},
        {0.0f,  0.0f, -1.0f},
        {0.0f,  0.0f, -1.0f},
        {0.0f,  0.0f, -1.0f},

        // Top face
        {0.0f,  1.0f,  0.0f},
        {0.0f,  1.0f,  0.0f},
        {0.0f,  1.0f,  0.0f},
        {0.0f,  1.0f,  0.0f},

        // Bottom face
        {0.0f, -1.0f,  0.0f},
        {0.0f, -1.0f,  0.0f},
        {0.0f, -1.0f,  0.0f},
        {0.0f, -1.0f,  0.0f},

        // Right face
        {1.0f,  0.0f,  0.0f},
        {1.0f,  0.0f,  0.0f},
        {1.0f,  0.0f,  0.0f},
        {1.0f,  0.0f,  0.0f},

        // Left face
        {-1.0f,  0.0f,  0.0f},
        {-1.0f,  0.0f,  0.0f},
        {-1.0f,  0.0f,  0.0f},
        {-1.0f,  0.0f,  0.0f}
    };

    std::vector<unsigned int> cubeIndices = {
        // Front
        0,  1,  2,
        2,  3,  0,
        // Back
        4,  5,  6,
        6,  7,  4,
        // Top
        8,  9, 10,
        10, 11,  8,
        // Bottom
        12, 13, 14,
        14, 15, 12,
        // Right
        16, 17, 18,
        18, 19, 16,
        // Left
        20, 21, 22,
        22, 23, 20
    };

    cubeIndexCount = cubeIndices.size();

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(glm::vec3) + cubeNormals.size() * sizeof(glm::vec3), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cubeVertices.size() * sizeof(glm::vec3), cubeVertices.data());
    glBufferSubData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(glm::vec3), cubeNormals.size() * sizeof(glm::vec3), cubeNormals.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), cubeIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(cubeVertices.size() * sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void WhiskeyGlassObject::update(Scene &scene, float dt) {
    Object::update(scene, dt);

    time += dt;

    //Bezier points animation
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
           // controlPoints[i][j]. = //animation;
        }
    }

    // New liquid point positions
    initLiquidCube();

    //Ice cubes animation
    for (auto& cube : iceCubes) {

    }
}

void WhiskeyGlassObject::render(Scene &scene) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    cubeShader->use();
    cubeShader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    cubeShader->setUniform("ViewMatrix", scene.camera->viewMatrix);

    cubeShader->setUniform("OverallColor", glm::vec3(0.8f));

    cubeShader->setUniform("LightDirection", glm::normalize(glm::vec3(5.0f, 5.0f, 5.0f)));
    cubeShader->setUniform("Transparency", 0.9f);

    glBindVertexArray(cubeVAO);

    for (const auto& cube : iceCubes) {
        glm::mat4 cubeLocalTransform = glm::mat4(1.0f);

        cubeLocalTransform = glm::translate(cubeLocalTransform, cube.position);
        cubeLocalTransform = glm::rotate(cubeLocalTransform, cube.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        cubeLocalTransform = glm::rotate(cubeLocalTransform, cube.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        cubeLocalTransform = glm::rotate(cubeLocalTransform, cube.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        cubeLocalTransform = glm::scale(cubeLocalTransform, cube.scale);

        glm::mat4 cubeWorldTransform = modelMatrix * cubeLocalTransform;

        cubeShader->setUniform("ModelMatrix", cubeWorldTransform);
        glDrawElements(GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    liquidShader->use();

    liquidShader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    liquidShader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    liquidShader->setUniform("ModelMatrix", modelMatrix);

    liquidShader->setUniform("OverallColor", glm::vec3(0.7f, 0.5f, 0.2f));

    liquidShader->setUniform("LightDirection", glm::normalize(glm::vec3(5.0f, 5.0f, 5.0f)));
    liquidShader->setUniform("Transparency", 0.6f);

    glBindVertexArray(liquidVAO);
    glDrawElements(GL_TRIANGLES, liquidIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    Object::render(scene);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}