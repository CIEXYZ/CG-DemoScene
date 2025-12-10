#pragma once

#include <GL/glew.h>
#include <ppgso/ppgso.h>
#include <memory>

enum class PostProcessingMode {
    NONE = 0,
    GRAYSCALE = 1,      // 1b - Basic per-pixel filter
    BLUR = 2,           // 2b - Convolution filter
    BLOOM = 3,          // 4b - Advanced bloom effect
    BLOOM_DEBUG = 4,    // Debug: show only bright areas
    SHADOW_DEBUG = 5    // Debug: show shadow map
};

class PostProcessing {
public:
    PostProcessing(int width, int height);
    ~PostProcessing();

    void beginFrame();
    void endFrame();
    void render();

    void setMode(PostProcessingMode mode) { currentMode = mode; }
    PostProcessingMode getMode() const { return currentMode; }

    void resize(int width, int height);

    // Set shadow map texture for debug visualization
    void setShadowMapTexture(GLuint texture) { shadowMapTexture = texture; }

    // Bloom parameters
    void setBloomThreshold(float threshold) { bloomThreshold = threshold; }
    void setBloomStrength(float strength) { bloomStrength = strength; }
    void setBloomBlurPasses(int passes) { bloomBlurPasses = passes; }

    // Bloom parameters
    float bloomThreshold = 0.8f;
    float bloomStrength = 0.3f;
    int bloomBlurPasses = 10;

private:
    void setupFramebuffers();
    void setupShaders();
    void setupQuad();

    void renderGrayscale();
    void renderBlur();
    void renderBloom();
    void renderBloomDebug();
    void renderShadowDebug();

    int width, height;
    PostProcessingMode currentMode = PostProcessingMode::NONE;

    // Main framebuffer (scene renders here)
    GLuint mainFBO;
    GLuint mainColorTexture;
    GLuint mainDepthRBO;

    // Ping-pong framebuffers for multi-pass effects
    GLuint pingpongFBO[2];
    GLuint pingpongTextures[2];

    // Shaders
    std::unique_ptr<ppgso::Shader> grayscaleShader;
    std::unique_ptr<ppgso::Shader> blurShader;
    std::unique_ptr<ppgso::Shader> bloomExtractShader;
    std::unique_ptr<ppgso::Shader> bloomCombineShader;

    // Screen quad
    GLuint quadVAO, quadVBO;

    // Shadow map texture for debug visualization
    GLuint shadowMapTexture = 0;
};