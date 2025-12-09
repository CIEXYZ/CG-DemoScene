#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ppgso/ppgso.h>
#include <memory>

class ShadowMap {
public:
    ShadowMap(int width = 2048, int height = 2048);
    ~ShadowMap();

    void beginShadowPass(const glm::vec3& lightDirection, const glm::vec3& sceneCenter, float sceneRadius);

    void endShadowPass();

    void bindShadowMap(int textureUnit = 1);

    glm::mat4 getLightSpaceMatrix() const { return lightSpaceMatrix; }

    // Get shadow map texture for debug visualization
    GLuint getShadowTexture() const { return shadowMap; }

    // Enable/disable shadows
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }

    void resize(int width, int height);

    std::shared_ptr<ppgso::Shader> getShadowShader() const { return shadowShader; }

private:
    void setupFramebuffer();
    void setupShader();
    
    int width, height;
    bool enabled = true;
    
    GLuint shadowFBO;
    GLuint shadowMap;
    
    glm::mat4 lightSpaceMatrix;
    
    std::shared_ptr<ppgso::Shader> shadowShader;
};