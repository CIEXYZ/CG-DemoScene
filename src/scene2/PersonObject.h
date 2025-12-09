#pragma once

#include "object.h"
#include "AnimationLoader.h"
#include "AnimatedMesh.h"
#include <memory>

class Crowd;

struct InstanceData {
    glm::mat4 modelMatrix;
    unsigned int textureIndex;
};

class PersonObject : public Object {
public:
    friend class Crowd;

    PersonObject(const std::string& modelPath);
    PersonObject(const std::string& modelPath, const std::string& animationPath);

    void loadAnimation(const std::string& animationPath);
    void playAnimation(int animationIndex = 0);

    void update(Scene &scene, float dt) override;
    void render(Scene &scene) override;

    // Instanced rendering
    void renderInstanced(Scene &scene, const std::vector<InstanceData>& instances);
    void setupInstanceBuffers(size_t maxInstances);
    void cleanupInstanceBuffers();

    Animator* getAnimator() { return animator.get(); }
    AnimatedMesh* getAnimMesh() { return animMesh.get(); }

private:
    std::unique_ptr<AnimatedMesh> animMesh;
    std::vector<std::unique_ptr<Animation>> animations;
    std::unique_ptr<Animator> animator;

    bool hasAnimation = false;

    // GPU Instancing buffers
    GLuint instanceMatrixVBO = 0;
    GLuint instanceTextureVBO = 0;
    size_t maxInstances = 0;
    bool instanceBuffersSetup = false;
};