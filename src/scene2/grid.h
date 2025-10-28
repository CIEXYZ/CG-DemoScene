#pragma once

#include <ppgso/ppgso.h>
#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>
#include <glm/glm.hpp>

class Grid {
private:
  std::unique_ptr<ppgso::Shader> shader;

  GLuint VAO, VBO, EBO;

  int gridIndexCount;
  int axesIndexCount;

public:
  Grid() {
    shader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    int gridSize = 20;
    float gridSpacing = 1.0f;
    unsigned int idx = 0;

    //Grid
    for (int i = -gridSize; i <= gridSize; i++) {
      if (i == 0) continue;

      //Drawing X-axis grid lines
      float z = i * gridSpacing;
      vertices.insert(vertices.end(), {
        -gridSize * gridSpacing, 0.0f, z, 0.3f, 0.3f, 0.3f,
         gridSize * gridSpacing, 0.0f, z, 0.3f, 0.3f, 0.3f
      });
      indices.push_back(idx++);
      indices.push_back(idx++);
    }

    //Drawing Z-axis grid lines
    for (int i = -gridSize; i <= gridSize; i++) {
      if (i == 0) continue;

      float x = i * gridSpacing;
      vertices.insert(vertices.end(), {
        x, 0.0f, -gridSize * gridSpacing, 0.3f, 0.3f, 0.3f,
        x, 0.0f, gridSize * gridSpacing, 0.3f, 0.3f, 0.3f
      });
      indices.push_back(idx++);
      indices.push_back(idx++);
    }
    gridIndexCount = indices.size();

    //Add colored axes lines
    auto add_axis = [&](glm::vec3 start, glm::vec3 end, glm::vec3 color) {
      vertices.insert(vertices.end(), {
        start.x, start.y, start.z, color.r, color.g, color.b,
        end.x, end.y, end.z, color.r, color.g, color.b
      });
      indices.push_back(idx++);
      indices.push_back(idx++);
    };

    add_axis({-gridSize * gridSpacing, 0, 0}, {gridSize * gridSpacing, 0, 0}, {1, 0, 0}); // X
    add_axis({0, 0, 0}, {0, gridSize * gridSpacing, 0}, {0, 1, 0});                      // Y
    add_axis({0, 0, -gridSize * gridSpacing}, {0, 0, gridSize * gridSpacing}, {0, 0, 1}); // Z

    axesIndexCount = indices.size() - gridIndexCount;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
  }

  ~Grid() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

  void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    glDisable(GL_DEPTH_TEST);

    shader->use();
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    shader->setUniform("ModelMatrix", modelMatrix);
    shader->setUniform("ViewMatrix", viewMatrix);
    shader->setUniform("ProjectionMatrix", projectionMatrix);

    glBindVertexArray(VAO);

    glLineWidth(1.0f);
    glDrawElements(GL_LINES, gridIndexCount, GL_UNSIGNED_INT, 0);

    glLineWidth(3.0f);
    glDrawElements(GL_LINES, axesIndexCount, GL_UNSIGNED_INT, (void*)(gridIndexCount * sizeof(unsigned int)));

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
  }
};