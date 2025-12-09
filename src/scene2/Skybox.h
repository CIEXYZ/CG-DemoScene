#pragma once
#include <iostream>
#include <shaders/skybox_vert_glsl.h>
#include <shaders/skybox_frag_glsl.h>

#include <ppgso/ppgso.h>

class Skybox {
    public:
    std::shared_ptr<ppgso::Shader> shader;
    GLuint VAO, VBO;
    GLuint cubemapTexture;

    Skybox(const std::vector<std::string>& faces);

    void render(const glm::mat4& view, const glm::mat4& projection);
};
