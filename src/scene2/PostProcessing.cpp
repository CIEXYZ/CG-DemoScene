#include "PostProcessing.h"
#include <iostream>

PostProcessing::PostProcessing(int width, int height) 
    : width(width), height(height) {
    
    setupFramebuffers();
    //setupShaders();
    setupQuad();
}

PostProcessing::~PostProcessing() {
    glDeleteFramebuffers(1, &mainFBO);
    glDeleteTextures(1, &mainColorTexture);
    glDeleteRenderbuffers(1, &mainDepthRBO);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongTextures);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

void PostProcessing::setupFramebuffers() {
    // Main framebuffer
    glGenFramebuffers(1, &mainFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mainFBO);
    
    // Color texture
    glGenTextures(1, &mainColorTexture);
    glBindTexture(GL_TEXTURE_2D, mainColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mainColorTexture, 0);
    
    // Depth renderbuffer
    glGenRenderbuffers(1, &mainDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mainDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mainDepthRBO);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Main framebuffer incomplete" << std::endl;
    }
    
    // Ping-pong framebuffers for blur/bloom
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongTextures);
    
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTextures[i], 0);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Ping-pong framebuffer " << i << " incomplete!" << std::endl;
        }
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// void PostProcessing::setupShaders() {
//     // Compile shaders
//     grayscaleShader = std::make_unique<ppgso::Shader>();
//
//     blurShader = std::make_unique<ppgso::Shader>(
//         std::vector<GLuint>{}, std::vector<GLuint>{}
//     );
//
//     bloomExtractShader = std::make_unique<ppgso::Shader>(
//         std::vector<GLuint>{}, std::vector<GLuint>{}
//     );
//
//     bloomCombineShader = std::make_unique<ppgso::Shader>(
//         std::vector<GLuint>{}, std::vector<GLuint>{}
//     );
//
//     // Manually compile (simplified - you may need to adapt to your Shader class)
//     // For now, we'll compile them when needed in render methods
// }

void PostProcessing::setupQuad() {
    float quadVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void PostProcessing::beginFrame() {
    // Bind main framebuffer - scene renders here
    glBindFramebuffer(GL_FRAMEBUFFER, mainFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessing::endFrame() {
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessing::render() {
    if (currentMode == PostProcessingMode::NONE) {
        // Just blit to screen without post-processing
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mainFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        return;
    }
    
    switch (currentMode) {
        case PostProcessingMode::GRAYSCALE:
            renderGrayscale();
            break;
        case PostProcessingMode::BLUR:
            renderBlur();
            break;
        case PostProcessingMode::BLOOM:
            renderBloom();
            break;
        default:
            break;
    }
}

void PostProcessing::renderGrayscale() {
    // Render to screen with grayscale shader
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use grayscale shader (you'll need to properly compile it)
    // For now, pseudocode:
    // grayscaleShader->use();
    // grayscaleShader->setUniform("screenTexture", mainColorTexture);
    
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mainColorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessing::renderBlur() {
    // Two-pass Gaussian blur
    bool horizontal = true;
    int passes = 10;  // More passes = more blur
    
    for (int i = 0; i < passes; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        
        // blurShader->use();
        // blurShader->setUniform("horizontal", horizontal);
        // blurShader->setUniform("screenTexture", i == 0 ? mainColorTexture : pingpongTextures[!horizontal]);
        
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, i == 0 ? mainColorTexture : pingpongTextures[!horizontal]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        horizontal = !horizontal;
    }
    
    // Final render to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, pingpongTextures[!horizontal]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessing::renderBloom() {
    // Step 1: Extract bright areas
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    // bloomExtractShader->use();
    // bloomExtractShader->setUniform("screenTexture", mainColorTexture);
    // bloomExtractShader->setUniform("threshold", 1.0f);
    glBindVertexArray(quadVAO);
    glBindTexture(GL_TEXTURE_2D, mainColorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Step 2: Blur bright areas (ping-pong)
    bool horizontal = true;
    for (int i = 0; i < 10; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        // blurShader->use();
        // blurShader->setUniform("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, pingpongTextures[!horizontal]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
    }
    
    // Step 3: Combine with original scene
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    // bloomCombineShader->use();
    // bloomCombineShader->setUniform("sceneTexture", mainColorTexture);
    // bloomCombineShader->setUniform("bloomTexture", pingpongTextures[!horizontal]);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessing::resize(int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    
    // Cleanup old framebuffers
    glDeleteFramebuffers(1, &mainFBO);
    glDeleteTextures(1, &mainColorTexture);
    glDeleteRenderbuffers(1, &mainDepthRBO);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongTextures);
    
    // Recreate with new size
    setupFramebuffers();
}