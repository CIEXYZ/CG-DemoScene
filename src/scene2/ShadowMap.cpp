#include "ShadowMap.h"
#include <iostream>

#include <shaders/shadow_vert_glsl.h>
#include <shaders/shadow_frag_glsl.h>

ShadowMap::ShadowMap(int width, int height) : width(width), height(height) {
    setupFramebuffer();
    setupShader();
}

ShadowMap::~ShadowMap() {
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteTextures(1, &shadowMap);
}

void ShadowMap::setupFramebuffer() {
    // Create framebuffer
    glGenFramebuffers(1, &shadowFBO);
    
    // Create depth texture
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow framebuffer incomplete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    std::cout << "Shadow map created: " << width << "x" << height << std::endl;
}

void ShadowMap::setupShader() {
    shadowShader = std::make_shared<ppgso::Shader>(shadow_vert_glsl, shadow_frag_glsl);
    std::cout << "Shadow shader loaded" << std::endl;
}

void ShadowMap::beginShadowPass(const glm::vec3& lightDirection, const glm::vec3& sceneCenter, float sceneRadius) {
    if (!enabled) return;
    
    // Calculate light view matrix
    glm::vec3 lightPos = sceneCenter - glm::normalize(lightDirection) * sceneRadius * 2.0f;
    glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Orthographic projection for directional light
    float orthoSize = sceneRadius * 0.5f;
    glm::mat4 lightProjection = glm::ortho(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        0.1f, sceneRadius * 4.0f
    );
    
    lightSpaceMatrix = lightProjection * lightView;
    
    // Bind shadow framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Enable face culling to reduce shadow acne
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    shadowShader->use();
    shadowShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
}

void ShadowMap::endShadowPass() {
    if (!enabled) return;
    
    // Restore face culling
    glCullFace(GL_BACK);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::bindShadowMap(int textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
}

void ShadowMap::resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    
    // Cleanup old framebuffer
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteTextures(1, &shadowMap);

    setupFramebuffer();
}