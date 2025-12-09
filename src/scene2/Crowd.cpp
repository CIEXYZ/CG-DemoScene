#include "Crowd.h"
#include "ModelLoader.h"
#include <random>
#include <iostream>

void Crowd::addPersonType(std::shared_ptr<PersonObject> person, const std::vector<std::string>& texturePaths) {
    PersonType personType;
    personType.personObject = person;

    // Load texture variations
    for (const auto& texPath : texturePaths) {
        loadTextureVariation(personType, texPath);
    }

    // If no textures provided, use the default texture
    if (personType.texturePaths.empty()) {
        personType.texturePaths.push_back("");
    }

    personTypes.push_back(personType);
}

void Crowd::loadTextureVariation(PersonType& personType, const std::string& texturePath) {
    auto texture = ModelLoader::loadTextureFile(texturePath);
    if (texture) {
        // Find a free material index
        unsigned int texIndex = personType.personObject->materialTextures.size();
        personType.personObject->materialTextures[texIndex] = texture;
        personType.texturePaths.push_back(texturePath);
        std::cout << "Loaded texture variation: " << texturePath << " at index " << texIndex << std::endl;
    }
}

void Crowd::generateCrowd(const PoissonDiskSampler::Rect& bounds, float minDistance, const glm::vec3& basePosition, const glm::vec3& baseScale, const PoissonDiskSampler::Rect* centerGap)
{
    if (personTypes.empty()) {
        std::cerr << "No person types added to crowd!" << std::endl;
        return;
    }

    members.clear();

    auto positions2D = PoissonDiskSampler::generate(bounds, minDistance, 30, centerGap);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> rotationDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> scaleDist(0.9f, 1.1f);
    std::uniform_int_distribution<int> personTypeDist(0, personTypes.size() - 1);

    for (const auto& pos2D : positions2D) {
        CrowdMember member;

        unsigned int personTypeIdx = personTypeDist(gen);
        member.personTypeIndex = personTypeIdx;

        // Random texture variation
        const auto& personType = personTypes[personTypeIdx];
        std::uniform_int_distribution<int> textureDist(0, personType.texturePaths.size() - 1);
        member.textureIndex = textureDist(gen);

        member.position = basePosition + glm::vec3(
            pos2D.x + bounds.x,
            0.0f,
            pos2D.y + bounds.y
        );

        member.rotation = glm::vec3(0.0f, rotationDist(gen), 0.0f);

        float scaleVar = scaleDist(gen);
        member.scale = baseScale * scaleVar;

        members.push_back(member);
    }

    std::cout << "Crowd generated with " << members.size() << " people" << std::endl;
}

void Crowd::addMember(unsigned int personTypeIndex, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, unsigned int textureIndex)
{
    if (personTypeIndex >= personTypes.size()) {
        std::cerr << "Invalid person type index!" << std::endl;
        return;
    }

    CrowdMember member;
    member.personTypeIndex = personTypeIndex;
    member.textureIndex = textureIndex;
    member.position = position;
    member.rotation = rotation;
    member.scale = scale;

    members.push_back(member);
}

void Crowd::update(Scene &scene, float dt) {
    for (auto& personType : personTypes) {
        if (personType.personObject->getAnimator()) {
            personType.personObject->getAnimator()->updateAnimation(dt);
        }
    }
}

void Crowd::render(Scene &scene) {
    // Group members by person type for instanced rendering
    std::map<unsigned int, std::vector<InstanceData>> instanceGroups;

    for (const auto& member : members) {
        InstanceData instanceData;

        // Build model matrix
        instanceData.modelMatrix = glm::mat4(1.0f);
        instanceData.modelMatrix = glm::translate(instanceData.modelMatrix, member.position);
        instanceData.modelMatrix = glm::rotate(instanceData.modelMatrix, member.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        instanceData.modelMatrix = glm::rotate(instanceData.modelMatrix, member.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        instanceData.modelMatrix = glm::rotate(instanceData.modelMatrix, member.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        instanceData.modelMatrix = glm::scale(instanceData.modelMatrix, member.scale);

        instanceData.textureIndex = member.textureIndex;

        instanceGroups[member.personTypeIndex].push_back(instanceData);
    }

    // Render each group
    for (const auto& [personTypeIdx, instances] : instanceGroups) {
        if (personTypeIdx < personTypes.size()) {
            personTypes[personTypeIdx].personObject->renderInstanced(scene, instances);
        }
    }
}