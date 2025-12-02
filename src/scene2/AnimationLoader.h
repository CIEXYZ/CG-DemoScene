#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "scene.h"
#include "Mesh_Assimp.h"

struct BoneInfo {
    int id;
    glm::mat4 offset;
};

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

class Bone {
public:
    Bone(const std::string& name, int id, const aiNodeAnim* channel);

    void update(float animationTime);

    glm::mat4 getLocalTransform() const { return localTransform; }
    std::string getBoneName() const { return name; }
    int getBoneID() const { return id; }

    int getPositionIndex(float animationTime);
    int getRotationIndex(float animationTime);
    int getScaleIndex(float animationTime);

private:
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    glm::mat4 interpolatePosition(float animationTime);
    glm::mat4 interpolateRotation(float animationTime);
    glm::mat4 interpolateScaling(float animationTime);

    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;
    int numPositions;
    int numRotations;
    int numScalings;

    glm::mat4 localTransform;
    std::string name;
    int id;
};

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation {
public:
    Animation() = default;
    Animation(const std::string& animationPath, const aiScene* scene);

    Bone* findBone(const std::string& name);

    float getTicksPerSecond() const { return ticksPerSecond; }
    float getDuration() const { return duration; }
    const std::map<std::string, BoneInfo>& getBoneInfoMap() const { return boneInfoMap; }
    const AssimpNodeData& getRootNode() const { return rootNode; }

private:
    void readMissingBones(const aiAnimation* animation, const aiScene* scene);
    void readHeirarchyData(AssimpNodeData& dest, const aiNode* src);

    float duration;
    int ticksPerSecond;
    std::vector<Bone> bones;
    std::map<std::string, BoneInfo> boneInfoMap;
    AssimpNodeData rootNode;
};

class Animator {
public:
    Animator(Animation* animation);

    void updateAnimation(float dt);
    void playAnimation(Animation* pAnimation);
    void calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

    std::vector<glm::mat4> getFinalBoneMatrices() const { return finalBoneMatrices; }

private:
    std::vector<glm::mat4> finalBoneMatrices;
    Animation* currentAnimation;
    float currentTime;
    float deltaTime;
};

class AnimationLoader {
public:
    static Animation loadAnimation(const std::string& animationPath, const aiScene* modelScene);
    static std::map<std::string, BoneInfo> extractBoneInfoMap(const aiScene* scene);

private:
    static void setBoneInfoMap(std::map<std::string, BoneInfo>& boneInfoMap, const aiScene* scene);
};