#include <iostream>
#include <algorithm>

#include "Timeline.h"
#include "scene.h"
#include "PersonObject.h"

void CameraMovementEvent::execute(Scene& scene) {
    if (duration == 0.0f || triggerTime == 0.0f) {
        scene.camera->position = targetPosition;
        scene.camera->yaw = targetRotation.x;
        scene.camera->pitch = targetRotation.y;

        scene.camera->updateRotation();
        scene.camera->update();
        isMoving = false;

        return;
    }

    startPosition = scene.camera->position;
    startRotation = glm::vec3(scene.camera->yaw, scene.camera->pitch, 0.0f);

    isMoving = true;
    elapsedTime = 0.0f;
}

static glm::vec3 cubicBezier(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t){
    glm::vec3 a = glm::mix(p0, p1, t);
    glm::vec3 b = glm::mix(p1, p2, t);
    glm::vec3 c = glm::mix(p2, p3, t);

    glm::vec3 d = glm::mix(a, b, t);
    glm::vec3 e = glm::mix(b, c, t);

    return glm::mix(d, e, t);
}

void CameraMovementEvent::update(Scene& scene, float dt) {
    if (!isMoving) return;

    elapsedTime += dt;
    float t = std::min(elapsedTime / duration, 1.0f);

    //Interpolation
    t = t * t * (3.0f - 2.0f * t);

    //Interpolate position
    scene.camera->position = cubicBezier(startPosition, crP1, crP2, targetPosition, t);

    //Interpolate rotation
    glm::vec3 currentRotation = startRotation + (targetRotation - startRotation) * t;
    scene.camera->yaw = currentRotation.x;
    scene.camera->pitch = currentRotation.y;
    scene.camera->updateRotation();

    if (t >= 1.0f) {
        isMoving = false;
    }
}

void CameraLookAtEvent::execute(Scene& scene) {
    startRotation = glm::vec3(scene.camera->yaw, scene.camera->pitch, 0.0f);

    isLooking = true;
    elapsedTime = 0.0f;
}

void CameraLookAtEvent::update(Scene& scene, float dt) {
    if (!isLooking) return;

    elapsedTime += dt;
    float t = std::min(elapsedTime / duration, 1.0f);

    //Interpolation
    t = t * t * (3.0f - 2.0f * t);

    // Calculate target rotation to look at position
    glm::vec3 direction = glm::normalize(lookAtPosition - scene.camera->position);
    float targetYaw = glm::degrees(atan2(direction.z, direction.x));
    float targetPitch = glm::degrees(asin(direction.y));

    // Interpolate rotation
    scene.camera->yaw = startRotation.x + (targetYaw - startRotation.x) * t;
    scene.camera->pitch = startRotation.y + (targetPitch - startRotation.y) * t;
    scene.camera->updateRotation();

    if (t >= 1.0f) {
        isLooking = false;
    }
}

void ObjectAnimationEvent::execute(Scene& scene) {
    for (auto& obj : scene.objects) {
        if (obj->name == objectName) {
            PersonObject* person = dynamic_cast<PersonObject*>(obj.get());
            if (person) {
                person->playAnimation(animationIndex);
                if (!description.empty()) {
                    std::cout << "[Timeline] " << description << std::endl;
                }
            }
            break;
        }
    }
}

void ObjectMovementEvent::execute(Scene& scene) {
    for (auto& obj : scene.objects) {
        if (obj->name == objectName) {
            startPosition = obj->position;
            startRotation = obj->rotation;

            isMoving = true;
            elapsedTime = 0.0f;

            if (!description.empty()) {
                std::cout << "[Timeline] " << description << std::endl;
            }
            break;
        }
    }
}

void ObjectMovementEvent::update(Scene& scene, float dt) {
    if (!isMoving) return;

    elapsedTime += dt;
    float t = std::min(elapsedTime / duration, 1.0f);

    //Interpolation
    t = t * t * (3.0f - 2.0f * t);

    for (auto& obj : scene.objects) {
        if (obj->name == objectName) {
            obj->position = startPosition + (targetPosition - startPosition) * t;
            obj->rotation = startRotation + (targetRotation - startRotation) * t;
            break;
        }
    }

    if (t >= 1.0f) {
        isMoving = false;
    }
}

void Timeline::addEvent(TimelineEvent* event) {
    events.push_back(event);

    // Sort events by trigger time
    std::sort(events.begin(), events.end(), [](TimelineEvent* a, TimelineEvent* b)
    {
        return a->triggerTime < b->triggerTime;
    });
}

void Timeline::update(Scene& scene, float dt) {
    if (paused) return;

    currentTime += dt * timeScale;

    // Check and trigger events
    for (auto* event : events) {
        if (!event->triggered && currentTime >= event->triggerTime) {
            event->triggered = true;
            event->execute(scene);

            if (auto* camMove = dynamic_cast<CameraMovementEvent*>(event)) {
                activeCameraMovements.push_back(camMove);
            }
            if (auto* camLook = dynamic_cast<CameraLookAtEvent*>(event)) {
                activeCameraLookAts.push_back(camLook);
            }
            if (auto* objMove = dynamic_cast<ObjectMovementEvent*>(event)) {
                activeObjectMovements.push_back(objMove);
            }
        }
    }

    // Update active camera movements
    for (auto it = activeCameraMovements.begin(); it != activeCameraMovements.end();) {
        (*it)->update(scene, dt * timeScale);
        if (!(*it)->isMoving) {
            it = activeCameraMovements.erase(it);
        } else {
            ++it;
        }
    }

    // Update active camera look-ats
    for (auto it = activeCameraLookAts.begin(); it != activeCameraLookAts.end();) {
        (*it)->update(scene, dt * timeScale);
        if (!(*it)->isLooking) {
            it = activeCameraLookAts.erase(it);
        } else {
            ++it;
        }
    }

    // Update active object movements
    for (auto it = activeObjectMovements.begin(); it != activeObjectMovements.end();) {
        (*it)->update(scene, dt * timeScale);
        if (!(*it)->isMoving) {
            it = activeObjectMovements.erase(it);
        } else {
            ++it;
        }
    }
}

void Timeline::initializeCameraPosition(Scene& scene) {
    for (auto* event : events) {
        if (event->triggerTime == 0.0f) {
            if (auto* camMove = dynamic_cast<CameraMovementEvent*>(event)) {
                // Set camera to the starting position
                scene.camera->position = camMove->targetPosition;
                scene.camera->yaw = camMove->targetRotation.x;
                scene.camera->pitch = camMove->targetRotation.y;

                scene.camera->updateRotation();
                scene.camera->update();
                return;
            }
        }
    }
}

void Timeline::initializeObjectPositions(Scene& scene) {
    for (auto* event : events) {
        if (event->triggerTime == 0.0f) {
            if (auto* objMove = dynamic_cast<ObjectMovementEvent*>(event)) {
                // Set object to starting position
                for (auto& obj : scene.objects) {
                    if (obj->name == objMove->objectName) {
                        obj->position = objMove->targetPosition;
                        obj->rotation = objMove->targetRotation;
                        break;
                    }
                }
            }
        }
    }
}

void Timeline::initializeAnimations(Scene& scene) {
    for (auto* event : events) {
        if (event->triggerTime == 0.0f) {
            if (auto* animEvent = dynamic_cast<ObjectAnimationEvent*>(event)) {
                // Set animation
                for (auto& obj : scene.objects) {
                    if (obj->name == animEvent->objectName) {
                        PersonObject* person = dynamic_cast<PersonObject*>(obj.get());
                        if (person) {
                            person->playAnimation(animEvent->animationIndex);
                        }
                        break;
                    }
                }
            }
        }
    }
}

void Timeline::reset() {
    currentTime = 0.0f;

    activeCameraMovements.clear();
    activeCameraLookAts.clear();
    activeObjectMovements.clear();

    for (auto* event : events) {
        event->triggered = false;
    }
}

void Timeline::pause() {
    paused = true;
    std::cout << "[Timeline] Paused at " << currentTime << "s" << std::endl;
}

void Timeline::resume() {
    paused = false;
    std::cout << "[Timeline] Resumed at " << currentTime << "s" << std::endl;
}

void Timeline::printUpcomingEvents(float lookAhead) const {
    std::cout << "\n=== Timeline Events (Current: " << currentTime << "s) ===" << std::endl;

    int count = 0;
    for (auto* event : events) {
        if (event->triggerTime > currentTime &&
            event->triggerTime <= currentTime + lookAhead) {
            std::cout << "[" << event->triggerTime << "s] "
                      << (event->description.empty() ? "Event" : event->description)
                      << std::endl;
            count++;
        }
    }

    if (count == 0) {
        std::cout << "No upcoming events in next " << lookAhead << "s" << std::endl;
    }

    std::cout << "==================================" << std::endl;
}