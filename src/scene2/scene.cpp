#include "scene.h"
#include <GLFW/glfw3.h>

void Scene::update(float dt) {
  updateCamera(dt);

  for (auto& obj : objects) {
    obj->update(*this, dt);
  }
}

void Scene::updateCamera(float dt) {
  // Camera movement
  glm::vec3 forward = -camera->back;
  glm::vec3 right = glm::normalize(glm::cross(forward, camera->up));

  float speed = camera->moveSpeed * dt;

  // WASD movement
  if (keyboard[GLFW_KEY_W]) {
    camera->position += forward * speed;
  }
  if (keyboard[GLFW_KEY_S]) {
    camera->position -= forward * speed;
  }
  if (keyboard[GLFW_KEY_A]) {
    camera->position -= right * speed;
  }
  if (keyboard[GLFW_KEY_D]) {
    camera->position += right * speed;
  }

  // Q/E for up/down
  if (keyboard[GLFW_KEY_Q]) {
    camera->position.y -= speed;
  }
  if (keyboard[GLFW_KEY_E]) {
    camera->position.y += speed;
  }

  // Arrow keys for rotation
  if (keyboard[GLFW_KEY_LEFT]) {
    camera->yaw -= camera->rotationSpeed * 100.0f * dt;
    camera->updateRotation();
  }
  if (keyboard[GLFW_KEY_RIGHT]) {
    camera->yaw += camera->rotationSpeed * 100.0f * dt;
    camera->updateRotation();
  }
  if (keyboard[GLFW_KEY_UP]) {
    camera->pitch += camera->rotationSpeed * 100.0f * dt;
    if (camera->pitch > 89.0f) camera->pitch = 89.0f;
    camera->updateRotation();
  }
  if (keyboard[GLFW_KEY_DOWN]) {
    camera->pitch -= camera->rotationSpeed * 100.0f * dt;
    if (camera->pitch < -89.0f) camera->pitch = -89.0f;
    camera->updateRotation();
  }

  camera->update();
}

void Scene::render() {
  for ( auto& obj : objects )
    obj->render(*this);
}

void Scene::renderGrid() {
  if (!grid) {
    grid = std::make_unique<Grid>();
  }

  grid->render(camera->viewMatrix, camera->projectionMatrix);
}