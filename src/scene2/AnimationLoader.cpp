#include "AnimationLoader.h"
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <assimp/postprocess.h>

#define MAX_BONE_INFLUENCE 4

//TODO: add more explanation to code

// Bone implementation
Bone::Bone(const std::string& name, int id, const aiNodeAnim* channel) : name(name), id(id), localTransform(1.0f) {
    numPositions = channel->mNumPositionKeys;

    for (int positionIndex = 0; positionIndex < numPositions; ++positionIndex) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;

        KeyPosition data;
        data.position = glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z);
        data.timeStamp = timeStamp;
        positions.push_back(data);
    }

    numRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < numRotations; ++rotationIndex) {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;

        KeyRotation data;
        data.orientation = glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z);
        data.timeStamp = timeStamp;
        rotations.push_back(data);
    }

    numScalings = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < numScalings; ++keyIndex) {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        float timeStamp = channel->mScalingKeys[keyIndex].mTime;

        KeyScale data;
        data.scale = glm::vec3(scale.x, scale.y, scale.z);
        data.timeStamp = timeStamp;
        scales.push_back(data);
    }
}

void Bone::update(float animationTime) {
    glm::mat4 translation = interpolatePosition(animationTime);
    glm::mat4 rotation = interpolateRotation(animationTime);
    glm::mat4 scale = interpolateScaling(animationTime);
    localTransform = translation * rotation * scale;
}

int Bone::getPositionIndex(float animationTime) {
    for (int index = 0; index < numPositions - 1; ++index) {
        if (animationTime < positions[index + 1].timeStamp)
            return index;
    }
    return 0;
}

int Bone::getRotationIndex(float animationTime) {
    for (int index = 0; index < numRotations - 1; ++index) {
        if (animationTime < rotations[index + 1].timeStamp)
            return index;
    }
    return 0;
}

int Bone::getScaleIndex(float animationTime) {
    for (int index = 0; index < numScalings - 1; ++index) {
        if (animationTime < scales[index + 1].timeStamp)
            return index;
    }
    return 0;
}

float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;

    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::mat4 Bone::interpolatePosition(float animationTime) {
    if (1 == numPositions)
        return glm::translate(glm::mat4(1.0f), positions[0].position);

    int p0Index = getPositionIndex(animationTime);
    int p1Index = p0Index + 1;

    float scaleFactor = getScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);

    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(float animationTime) {
    if (1 == numRotations) {
        auto rotation = glm::normalize(rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = getRotationIndex(animationTime);
    int p1Index = p0Index + 1;

    float scaleFactor = getScaleFactor(rotations[p0Index].timeStamp, rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation, rotations[p1Index].orientation, scaleFactor);

    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScaling(float animationTime) {
    if (1 == numScalings)
        return glm::scale(glm::mat4(1.0f), scales[0].scale);

    int p0Index = getScaleIndex(animationTime);
    int p1Index = p0Index + 1;

    float scaleFactor = getScaleFactor(scales[p0Index].timeStamp, scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(scales[p0Index].scale, scales[p1Index].scale, scaleFactor);

    return glm::scale(glm::mat4(1.0f), finalScale);
}

// Animation implementation
Animation::Animation(const std::string& animationPath, const aiScene* scene) {
    Assimp::Importer importer;
    const aiScene* animScene = importer.ReadFile(animationPath, aiProcess_Triangulate);

    if (!animScene || !animScene->mRootNode || !animScene->mNumAnimations) {
        std::cerr << "Failed to load animation: " << animationPath << std::endl;
        return;
    }

    auto animation = animScene->mAnimations[0];
    duration = animation->mDuration;
    ticksPerSecond = animation->mTicksPerSecond;

    std::cout << "Animation loaded: " << animation->mName.C_Str() << std::endl;
    std::cout << "Duration: " << duration << " ticks" << std::endl;
    std::cout << "Ticks per second: " << ticksPerSecond << std::endl;
    std::cout << "Number of channels: " << animation->mNumChannels << std::endl;

    readHeirarchyData(rootNode, animScene->mRootNode);
    readMissingBones(animation, scene);
}

Bone* Animation::findBone(const std::string& name) {
    auto iter = std::find_if(bones.begin(), bones.end(),
        [&](const Bone& bone) {
            return bone.getBoneName() == name;
        }
    );
    if (iter == bones.end()) return nullptr;

    return &(*iter);
}

void Animation::readMissingBones(const aiAnimation* animation, const aiScene* scene) {
    int size = animation->mNumChannels;

    // Get bone info from model
    auto boneInfoMap = AnimationLoader::extractBoneInfoMap(scene);
    int boneCount = boneInfoMap.size();

    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
    }

    this->boneInfoMap = boneInfoMap;
}

void Animation::readHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
    dest.name = src->mName.data;
    dest.transformation = glm::transpose(glm::make_mat4(&src->mTransformation.a1));
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        readHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

// Animator implementation
Animator::Animator(Animation* animation) {
    currentTime = 0.0;
    currentAnimation = animation;

    finalBoneMatrices.reserve(100);
    for (int i = 0; i < 100; i++)
        finalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::updateAnimation(float dt) {
    deltaTime = dt;
    if (currentAnimation) {
        currentTime += currentAnimation->getTicksPerSecond() * dt;
        currentTime = fmod(currentTime, currentAnimation->getDuration());
        calculateBoneTransform(&currentAnimation->getRootNode(), glm::mat4(1.0f));
    }
}

void Animator::playAnimation(Animation* pAnimation) {
    currentAnimation = pAnimation;
    currentTime = 0.0f;
}

void Animator::calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* bone = currentAnimation->findBone(nodeName);

    if (bone) {
        bone->update(currentTime);
        nodeTransform = bone->getLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = currentAnimation->getBoneInfoMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        finalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        calculateBoneTransform(&node->children[i], globalTransformation);
}

// AnimationLoader implementation
Animation AnimationLoader::loadAnimation(const std::string& animationPath, const aiScene* modelScene) {
    return Animation(animationPath, modelScene);
}

std::map<std::string, BoneInfo> AnimationLoader::extractBoneInfoMap(const aiScene* scene) {
    std::map<std::string, BoneInfo> boneInfoMap;
    setBoneInfoMap(boneInfoMap, scene);
    return boneInfoMap;
}

void AnimationLoader::setBoneInfoMap(std::map<std::string, BoneInfo>& boneInfoMap, const aiScene* scene) {
    int boneCount = 0;

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            aiBone* bone = mesh->mBones[i];
            std::string boneName = bone->mName.C_Str();

            if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = glm::transpose(glm::make_mat4(&bone->mOffsetMatrix.a1));
                boneInfoMap[boneName] = newBoneInfo;
                boneCount++;
            }
        }
    }

    std::cout << "Total bones found: " << boneCount << std::endl;
}