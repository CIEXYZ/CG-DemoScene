#include "DanceFloorObject.h"
#include "scene.h"
#include "ModelLoader.h"

#include <GL/glew.h>

#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>

DanceFloorObject::DanceFloorObject() {
    shader = std::make_unique<ppgso::Shader>(diffuse_vert_glsl, diffuse_frag_glsl);
    texture = ModelLoader::loadTextureFile("models/disco_ball/textures/dancefloor_texture.png");

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
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        // Back face
        0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f, -0.5f,
       -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,

        // Left face
       -0.5f, -0.5f, -0.5f,
       -0.5f, -0.5f,  0.5f,
       -0.5f,  0.5f,  0.5f,
       -0.5f,  0.5f, -0.5f,

        // Right face
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,

        // Top face
       -0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
       -0.5f,  0.5f, -0.5f,

        // Bottom face
       -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
       -0.5f, -0.5f,  0.5f,
    };

    float texCoords[] = {
        // Front
        0,0,
        1,0,
        1,1,
        0,1,
        // Back
        0,0,
        1,0,
        1,1,
        0,1,
        // Left
        0,0,
        1,0,
        1,1,
        0,1,
        // Right
        0,0,
        1,0,
        1,1,
        0,1,
        // Top
        0,0,
        1,0,
        1,1,
        0,1,
        // Bottom
        0,0,
        1,0,
        1,1,
        0,1
    };

    float normals[] = {
        // Front
        0,0,1,
        0,0,1,
        0,0,1,
        0,0,1,
        // Back
        0,0,-1,
        0,0,-1,
        0,0,-1,
        0,0,-1,
        // Left
        -1,0,0,
        -1,0,0,
        -1,0,0,
        -1,0,0,
        // Right
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        // Top
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0,
        // Bottom
        0,-1,0,
        0,-1,0,
        0,-1,0,
        0,-1,0,
    };

    unsigned int indices[] = {
        0,1,2,
        2,3,0,       // Front

        4,5,6,
        6,7,4,       // Back

        8,9,10,
        10,11,8,    // Left
        12,13,14,
        14,15,12, // Right
        16,17,18, 18,19,16, // Top
        20,21,22, 22,23,20  // Bottom
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

    glm::vec3 blueTint = glm::vec3(0.0f, 0.0f, 1.0f);   // Orange
    float maxDistance = 2.0f; // Maximum distance for color transition

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            // Calculate world position of this tile
            glm::vec3 tileWorldPos = glm::vec3(modelMatrix * glm::vec4(gridCubes[x][z].position, 1.0f));

            // Calculate distance from camera to tile
            float distance = glm::length(scene.camera->position - tileWorldPos);

            float normalizedDistance = glm::clamp(distance / maxDistance, 0.0f, 1.0f);

            // Mix the random color with orange based on camera distance
            cubeColors[x][z] = glm::mix(blueTint, cubeColors[x][z], normalizedDistance);
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
    shader->setUniform("usePointLight", scene.usePointLight);
    if (scene.usePointLight) {
        shader->setUniform("pointRedLight.position", scene.redLight.position);
        shader->setUniform("pointRedLight.ambient", scene.redLight.ambient);
        shader->setUniform("pointRedLight.diffuse", scene.redLight.diffuse);
        shader->setUniform("pointRedLight.specular", scene.redLight.specular);
        shader->setUniform("pointRedLight.constant", scene.redLight.constant);
        shader->setUniform("pointRedLight.linear", scene.redLight.linear);
        shader->setUniform("pointRedLight.quadratic", scene.redLight.quadratic);

        shader->setUniform("pointGreenLight.position", scene.greenLight.position);
        shader->setUniform("pointGreenLight.ambient", scene.greenLight.ambient);
        shader->setUniform("pointGreenLight.diffuse", scene.greenLight.diffuse);
        shader->setUniform("pointGreenLight.specular", scene.greenLight.specular);
        shader->setUniform("pointGreenLight.constant", scene.greenLight.constant);
        shader->setUniform("pointGreenLight.linear", scene.greenLight.linear);
        shader->setUniform("pointGreenLight.quadratic", scene.greenLight.quadratic);

        shader->setUniform("pointBlueLight.position", scene.blueLight.position);
        shader->setUniform("pointBlueLight.ambient", scene.blueLight.ambient);
        shader->setUniform("pointBlueLight.diffuse", scene.blueLight.diffuse);
        shader->setUniform("pointBlueLight.specular", scene.blueLight.specular);
        shader->setUniform("pointBlueLight.constant", scene.blueLight.constant);
        shader->setUniform("pointBlueLight.linear", scene.blueLight.linear);
        shader->setUniform("pointBlueLight.quadratic", scene.blueLight.quadratic);

        shader->setUniform("pointCeilingLight.position", scene.ceilingLight.position);
        shader->setUniform("pointCeilingLight.ambient", scene.ceilingLight.ambient);
        shader->setUniform("pointCeilingLight.diffuse", scene.ceilingLight.diffuse);
        shader->setUniform("pointCeilingLight.specular", scene.ceilingLight.specular);
        shader->setUniform("pointCeilingLight.constant", scene.ceilingLight.constant);
        shader->setUniform("pointCeilingLight.linear", scene.ceilingLight.linear);
        shader->setUniform("pointCeilingLight.quadratic", scene.ceilingLight.quadratic);
    }

    //SPOTLIGHTS
    shader->setUniform("useSpotLight", scene.useSpotLight);
    if (scene.useSpotLight) {
        shader->setUniform("spotDJLights.position", scene.djSpot.position);
        shader->setUniform("spotDJLights.direction", scene.djSpot.direction);
        shader->setUniform("spotDJLights.ambient", scene.djSpot.ambient);
        shader->setUniform("spotDJLights.diffuse", scene.djSpot.diffuse);
        shader->setUniform("spotDJLights.specular", scene.djSpot.specular);
        shader->setUniform("spotDJLights.cutOff", scene.djSpot.cutOff);
        shader->setUniform("spotDJLights.outerCutOff", scene.djSpot.outerCutOff);
        shader->setUniform("spotDJLights.constant", scene.djSpot.constant);
        shader->setUniform("spotDJLights.linear", scene.djSpot.linear);
        shader->setUniform("spotDJLights.quadratic", scene.djSpot.quadratic);

        shader->setUniform("spotDanceLights.position", scene.danceSpot.position);
        shader->setUniform("spotDanceLights.direction", scene.danceSpot.direction);
        shader->setUniform("spotDanceLights.ambient", scene.danceSpot.ambient);
        shader->setUniform("spotDanceLights.diffuse", scene.danceSpot.diffuse);
        shader->setUniform("spotDanceLights.specular", scene.danceSpot.specular);
        shader->setUniform("spotDanceLights.cutOff", scene.danceSpot.cutOff);
        shader->setUniform("spotDanceLights.outerCutOff", scene.danceSpot.outerCutOff);
        shader->setUniform("spotDanceLights.constant", scene.danceSpot.constant);
        shader->setUniform("spotDanceLights.linear", scene.danceSpot.linear);
        shader->setUniform("spotDanceLights.quadratic", scene.danceSpot.quadratic);
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