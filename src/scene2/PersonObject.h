#pragma once

#include "object.h"
#include "AnimationLoader.h"
#include "AnimatedMesh.h"
#include <memory>

class PersonObject : public Object {
public:
    PersonObject(const std::string& modelPath);
    PersonObject(const std::string& modelPath, const std::string& animationPath);

    void loadAnimation(const std::string& animationPath);
    void playAnimation(int animationIndex = 0);

    void update(Scene &scene, float dt) override;
    void render(Scene &scene) override;

    Animator* getAnimator() { return animator.get(); }

private:
    std::unique_ptr<AnimatedMesh> animMesh;
    std::vector<std::unique_ptr<Animation>> animations;
    std::unique_ptr<Animator> animator;
    
    bool hasAnimation = false;
};