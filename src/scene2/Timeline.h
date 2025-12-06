#include <vector>
#include <functional>
#include <string>
#include <glm/glm.hpp>

class Scene;

struct TimelineEvent {
    float triggerTime;
    bool triggered = false;
    std::string description;

    TimelineEvent(float time, const std::string& desc = "") : triggerTime(time), description(desc) {}

    virtual ~TimelineEvent() = default;
    virtual void execute(Scene& scene) = 0;
};

// Camera movement event
struct CameraMovementEvent : public TimelineEvent {
    glm::vec3 startPosition;
    glm::vec3 startRotation;

    glm::vec3 crP1;
    glm::vec3 crP2;

    glm::vec3 targetPosition;
    glm::vec3 targetRotation;

    float duration;
    float elapsedTime = 0.0f;

    bool isMoving = false;

    CameraMovementEvent(float time, const glm::vec3& pos, const glm::vec3& crP1, const glm::vec3& crP2, const glm::vec3& rot, float dur, const std::string& desc = "")
        : TimelineEvent(time, desc), targetPosition(pos), crP1(crP1), crP2(crP2), targetRotation(rot), duration(dur) {}

    void execute(Scene& scene) override;
    void update(Scene& scene, float dt);
};

// Camera look at event
struct CameraLookAtEvent : public TimelineEvent {
    glm::vec3 startRotation;
    glm::vec3 lookAtPosition;

    float duration;
    float elapsedTime = 0.0f;

    bool isLooking = false;

    CameraLookAtEvent(float time, const glm::vec3& lookAt, float dur, const std::string& desc = "") : TimelineEvent(time, desc), lookAtPosition(lookAt), duration(dur) {}

    void execute(Scene& scene) override;
    void update(Scene& scene, float dt);
};

// Object animation event
struct ObjectAnimationEvent : public TimelineEvent {
    std::string objectName;
    int animationIndex;

    ObjectAnimationEvent(float time, const std::string& name, int animIdx, const std::string& desc = "") : TimelineEvent(time, desc), objectName(name), animationIndex(animIdx) {}

    void execute(Scene& scene) override;
};

// Object movement event
struct ObjectMovementEvent : public TimelineEvent {
    std::string objectName;

    glm::vec3 startPosition;
    glm::vec3 targetPosition;

    glm::vec3 startRotation;
    glm::vec3 targetRotation;

    float duration;
    float elapsedTime = 0.0f;

    bool isMoving = false;

    ObjectMovementEvent(float time, const std::string& name, const glm::vec3& pos, const glm::vec3& rot, float dur, const std::string& desc = "")
        : TimelineEvent(time, desc), objectName(name), targetPosition(pos), targetRotation(rot), duration(dur) {}

    void execute(Scene& scene) override;
    void update(Scene& scene, float dt);
};

// Generic callback event
struct CallbackEvent : public TimelineEvent {
    std::function<void(Scene&)> callback;

    CallbackEvent(float time, std::function<void(Scene&)> cb, const std::string& desc = "") : TimelineEvent(time, desc), callback(cb) {}

    void execute(Scene& scene) override { if (callback) callback(scene); }
};

// Timeline manager
class Timeline {
public:
    void addEvent(TimelineEvent* event);

    void update(Scene& scene, float dt);
    void reset();
    void pause();
    void resume();

    void setTimeScale(float scale) { timeScale = scale; }
    float getCurrentTime() const { return currentTime; }

    bool isPaused() const { return paused; }

    void printUpcomingEvents(float lookAhead = 10.0f) const;

    void initializeCameraPosition(Scene& scene);
    void initializeObjectPositions(Scene& scene);
    void initializeAnimations(Scene& scene);

private:
    std::vector<TimelineEvent*> events;
    std::vector<CameraMovementEvent*> activeCameraMovements;
    std::vector<CameraLookAtEvent*> activeCameraLookAts;
    std::vector<ObjectMovementEvent*> activeObjectMovements;

    float currentTime = 0.0f;
    float timeScale = 1.0f;
    bool paused = false;
};