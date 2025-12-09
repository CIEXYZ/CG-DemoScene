#include "PersonObject.h"
#include "PoissonDiskSampler.h"
#include <memory>
#include <vector>

struct CrowdMember {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    unsigned int personTypeIndex;
    unsigned int textureIndex;
};

struct PersonType {
    std::shared_ptr<PersonObject> personObject;
    std::vector<std::string> texturePaths;
};

class Crowd : public Object {
public:
    Crowd() = default;

    // Add a person type with optional texture variations
    void addPersonType(std::shared_ptr<PersonObject> person, const std::vector<std::string>& texturePaths = {});

    // Generate crowd using Poisson disk sampling
    void generateCrowd(
        const PoissonDiskSampler::Rect& bounds,
        float minDistance,
        const glm::vec3& basePosition,
        const glm::vec3& baseScale,
        const PoissonDiskSampler::Rect* centerGap = nullptr
    );

    // Manually add a person at specific position
    void addMember(
        unsigned int personTypeIndex,
        const glm::vec3& position,
        const glm::vec3& rotation,
        const glm::vec3& scale,
        unsigned int textureIndex = 0
    );

    void update(Scene &scene, float dt) override;
    void render(Scene &scene) override;

    size_t getMemberCount() const { return members.size(); }

private:
    std::vector<PersonType> personTypes;
    std::vector<CrowdMember> members;

    void loadTextureVariation(PersonType& personType, const std::string& texturePath);
};

