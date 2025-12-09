#pragma once

#include <GL/glew.h>
#include <ppgso/ppgso.h>
#include <memory>

enum class PostProcessingMode {
    NONE = 0,
    GRAYSCALE = 1,      // 1b - Basic per-pixel filter
    BLUR = 2,           // 2b - Convolution filter
    BLOOM = 3           // 4b - Advanced bloom effect
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

private:
    void setupFramebuffers();
    void setupShaders();
    void setupQuad();

    void renderGrayscale();
    void renderBlur();
    void renderBloom();

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
};