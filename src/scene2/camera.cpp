#include <glm/glm.hpp>

#include "camera.h"

Camera::Camera(float fow, float ratio, float near, float far) {
  float fowInRad = (ppgso::PI/180.0f) * fow;

  projectionMatrix = glm::perspective(fowInRad, ratio, near, far);
}

void Camera::update() {
  viewMatrix = lookAt(position, position-back, up);
}

void Camera::updateRotation() {
  // Update camera direction based on yaw and pitch
  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  back = -glm::normalize(direction);
}

glm::vec3 Camera::cast(double u, double v) {
  glm::vec4 screenPosition{u,v,0.0f,1.0f};

  auto invProjection = glm::inverse(projectionMatrix);
  auto invView = glm::inverse(viewMatrix);

  auto planePosition = invView * invProjection * screenPosition;
  planePosition /= planePosition.w;

  auto direction = glm::normalize(planePosition - glm::vec4{position,1.0f});
  return glm::vec3{direction};
}