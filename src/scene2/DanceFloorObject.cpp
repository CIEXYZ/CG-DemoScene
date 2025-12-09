#include "DanceFloorObject.h"
#include "scene.h"
#include "ModelLoader.h"

#include <GL/glew.h>

#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>

DanceFloorObject::DanceFloorObject() {
    shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    texture = ModelLoader::createWhiteTexture();

    init();
    initGrid();
}

DanceFloorObject::~DanceFloorObject() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &TBO);
}

void DanceFloorObject::init() {
    //Create a single cube geometry that will be reused for all cubes and platform
    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  // 0
         0.5f, -0.5f,  0.5f,  // 1
         0.5f,  0.5f,  0.5f,  // 2
        -0.5f,  0.5f,  0.5f,  // 3
        // Back face
        -0.5f, -0.5f, -0.5f,  // 4
         0.5f, -0.5f, -0.5f,  // 5
         0.5f,  0.5f, -0.5f,  // 6
        -0.5f,  0.5f, -0.5f   // 7
    };

    float texCoords[] = {
        // Front face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        // Back face
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    float normals[] = {
        // Front face
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
       // Back face
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f
   };

    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        // Back face
        5, 4, 7,
        7, 6, 5,
        // Left face
        4, 0, 3,
        3, 7, 4,
        // Right face
        1, 5, 6,
        6, 2, 1,
        // Top face
        3, 2, 6,
        6, 7, 3,
        // Bottom face
        4, 5, 1,
        1, 0, 4
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &TBO);
    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glGenBuffers(1, &NBO);
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void DanceFloorObject::initGrid() {
    // Initialize the 12x12 grid
    cubeColors.resize(GRID_SIZE);
    gridCubes.resize(GRID_SIZE);

    float cubeSize = 1.0f;
    float spacing = 0.05f;
    float totalSize = cubeSize + spacing;

    float gridOffset = (GRID_SIZE - 1) * totalSize / 2.0f;

    for (int x = 0; x < GRID_SIZE; x++) {
        cubeColors[x].resize(GRID_SIZE);
        gridCubes[x].resize(GRID_SIZE);

        for (int z = 0; z < GRID_SIZE; z++) {
            cubeColors[x][z] = glm::vec3(1.0f, 1.0f, 1.0f);

            // Precomputing position and scale
            gridCubes[x][z].position = glm::vec3(
                x * totalSize - gridOffset,
                0.1f,
                z * totalSize - gridOffset
            );
            gridCubes[x][z].scale = glm::vec3(cubeSize, 0.1f, cubeSize);
        }
    }
    float platformHeight = 0.1f;
    float platformSize = GRID_SIZE * 1.1f;

    platform.position = glm::vec3(0.0f, -platformHeight / 2.0f, 0.0f);
    platform.scale = glm::vec3(platformSize, platformHeight, platformSize);
}

void DanceFloorObject::update(Scene &scene, float dt) {
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, scale);

    //Predefined colors for dance floor
    static const glm::vec3 colorPalette[] = {
        glm::vec3(1.0f, 0.0f, 1.0f),   //Pink
        glm::vec3(0.0f, 1.0f, 0.0f),   //Green
        glm::vec3(0.0f, 1.0f, 1.0f),   //Blue
        glm::vec3(1.0f, 1.0f, 0.0f),   //Yellow
        glm::vec3(1.0f, 1.0f, 1.0f)    //White
    };
    static const int paletteSize = sizeof(colorPalette) / sizeof(colorPalette[0]);

    //Random color selection from palette
    static float colorChangeTimer = 0.0f;
    colorChangeTimer += dt;

    // Change colors every N seconds
    if (colorChangeTimer >= 0.5f) {
        colorChangeTimer = 0.0f;

        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                // Pick a random color from the array
                int randomIndex = rand() % paletteSize;
                cubeColors[x][z] = colorPalette[randomIndex];
            }
        }
    }
}

void DanceFloorObject::render(Scene &scene) {
    shader->use();

    //BASIC
    shader->setUniform("ProjectionMatrix", scene.camera->projectionMatrix);
    shader->setUniform("ViewMatrix", scene.camera->viewMatrix);
    shader->setUniform("ModelMatrix", modelMatrix);

    //MATERIAL PROPERTIES
    shader->setUniform("material.ambient", scene.defaultMaterial.ambient);
    shader->setUniform("material.diffuse", scene.defaultMaterial.diffuse);
    shader->setUniform("material.specular", scene.defaultMaterial.specular);
    shader->setUniform("material.shininess", scene.defaultMaterial.shininess);

    //CAMERA POSITION
    shader->setUniform("viewPos", scene.camera->position);

    //DIRECTIONAL
    shader->setUniform("useDirLight", scene.useDirLight);
    if (scene.useDirLight) {
        shader->setUniform("dirLight.direction", scene.directionalLight.direction);
        shader->setUniform("dirLight.ambient", scene.directionalLight.ambient);
        shader->setUniform("dirLight.diffuse", scene.directionalLight.diffuse);
        shader->setUniform("dirLight.specular", scene.directionalLight.specular);
    }
    //POINT
    shader->setUniform("numPointLights", (int)scene.pointLights.size());
    for (size_t i = 0; i < scene.pointLights.size() && i < 10; i++) {
        std::string base = "pointLights[" + std::to_string(i) + "]";
        shader->setUniform(base + ".position", scene.pointLights[i].position);
        shader->setUniform(base + ".ambient", scene.pointLights[i].ambient);
        shader->setUniform(base + ".diffuse", scene.pointLights[i].diffuse);
        shader->setUniform(base + ".specular", scene.pointLights[i].specular);
        shader->setUniform(base + ".constant", scene.pointLights[i].constant);
        shader->setUniform(base + ".linear", scene.pointLights[i].linear);
        shader->setUniform(base + ".quadratic", scene.pointLights[i].quadratic);
    }

    //SPOTLIGHTS
    shader->setUniform("numSpotLights", (int)scene.spotLights.size());
    for (size_t i = 0; i < scene.spotLights.size() && i < 5; i++) {
        std::string base = "spotLights[" + std::to_string(i) + "]";
        shader->setUniform(base + ".position", scene.spotLights[i].position);
        shader->setUniform(base + ".direction", scene.spotLights[i].direction);
        shader->setUniform(base + ".ambient", scene.spotLights[i].ambient);
        shader->setUniform(base + ".diffuse", scene.spotLights[i].diffuse);
        shader->setUniform(base + ".specular", scene.spotLights[i].specular);
        shader->setUniform(base + ".cutOff", scene.spotLights[i].cutOff);
        shader->setUniform(base + ".outerCutOff", scene.spotLights[i].outerCutOff);
        shader->setUniform(base + ".constant", scene.spotLights[i].constant);
        shader->setUniform(base + ".linear", scene.spotLights[i].linear);
    }

     //LIGHTING OPTIONS
    shader->setUniform("useBlinnPhong", scene.useBlinnPhong);

    shader->setUniform("Transparency", 1.0f);
    shader->setUniform("TextureOffset", glm::vec2(0.0f, 0.0f));
    shader->setUniform("Texture", *texture);

    glBindVertexArray(VAO);

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            glm::mat4 cubeModelMatrix = modelMatrix;
            cubeModelMatrix = glm::translate(cubeModelMatrix, gridCubes[x][z].position);
            cubeModelMatrix = glm::scale(cubeModelMatrix, gridCubes[x][z].scale);

            shader->setUniform("ModelMatrix", cubeModelMatrix);
            shader->setUniform("ColorTint", cubeColors[x][z]);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
    }

    glm::mat4 platformModelMatrix = modelMatrix;
    platformModelMatrix = glm::translate(platformModelMatrix, platform.position);
    platformModelMatrix = glm::scale(platformModelMatrix, platform.scale);

    shader->setUniform("ModelMatrix", platformModelMatrix);
    shader->setUniform("ColorTint", glm::vec3(0.3f, 0.3f, 0.3f)); // Dark gray

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}