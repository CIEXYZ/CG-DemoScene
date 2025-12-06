#include "scene.h"
#include "Timeline.h"
#include <GLFW/glfw3.h>

void Scene::update(float dt) {
  if (useTimeline && timeline) {
    timeline->update(*this, dt);
  }
  if (allowManualCamera || !useTimeline) {
    updateCamera(dt);
  }
  else {
    camera->update();
  }

  for (auto& obj : objects) {
    obj->update(*this, dt);
  }

  updateLights(dt);

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

void Scene::setupLighting() {

    // DEFAULT MATERIAL
    defaultMaterial.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    defaultMaterial.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    defaultMaterial.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    defaultMaterial.shininess = 32.0f;


    // DIRECTIONAL LIGHT
    directionalLight.direction = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
    directionalLight.ambient = glm::vec3(0.15f, 0.15f, 0.2f);   // Slight blue ambient
    directionalLight.diffuse = glm::vec3(0.4f, 0.4f, 0.5f);     // Soft diffuse
    directionalLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);


    //POINT LIGHTS (Bar/Club colored lights)
    auto spotLight = building->getMesh()->getMeshBounds("SwivelLightGlow2_M_LightsMediumProps_0");
    glm::vec3 spotLightPosition = building->position + (spotLight.center * building->scale);
    // RED light
    PointLight redLight;
    redLight.position = spotLightPosition;
    redLight.ambient = glm::vec3(0.2f, 0.0f, 0.0f);
    redLight.diffuse = glm::vec3(1.0f, 0.2f, 0.2f);  // Bright red
    redLight.specular = glm::vec3(1.0f, 0.2f, 0.2f);
    redLight.setDistance(50);
    pointLights.push_back(redLight);

  // TEST LIGHT - Simple bright light at origin for debugging
  PointLight testLight;
  testLight.position = glm::vec3(0.0f, 2.0f, 0.0f);
  testLight.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
  testLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);  // White light
  testLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
  testLight.setDistance(20);
  pointLights.push_back(testLight);

  std::cout << "Point lights count: " << pointLights.size() << std::endl;

    //
    // PointLight greenLight;
    // greenLight.position = spotLightPosition;//glm::vec3(-3.2f, 2.0f, 0.0f);
    // greenLight.ambient = glm::vec3(0.0f, 0.1f, 0.0f);
    // greenLight.diffuse = glm::vec3(0.2f, 1.0f, 0.2f);  // Bright green
    // greenLight.specular = glm::vec3(0.2f, 1.0f, 0.2f);
    // greenLight.setDistance(20);
    // pointLights.push_back(greenLight);
    //
    //
    // PointLight blueLight;
    // blueLight.position =spotLightPosition + glm::vec3(2.f, 0.0f, 0.0f);// glm::vec3(-3.2f, 2.0f, 0.0f);
    // blueLight.ambient = glm::vec3(0.0f, 0.0f, 0.1f);
    // blueLight.diffuse = glm::vec3(0.2f, 0.2f, 1.0f);  // Bright blue
    // blueLight.specular = glm::vec3(0.2f, 0.2f, 1.0f);
    // blueLight.setDistance(20);
    // pointLights.push_back(blueLight);

    //
    // PointLight ceilingLight;
    // ceilingLight.position = glm::vec3(0.0f, 4.5f, 2.0f);
    // ceilingLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    // ceilingLight.diffuse = glm::vec3(0.9f, 0.85f, 0.7f);  // Warm white
    // ceilingLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    // ceilingLight.setDistance(50);  // Large coverage
    // pointLights.push_back(ceilingLight);
    //
    // SPOTLIGHTS
    // DJ Spotlight
    SpotLight djSpot;
    djSpot.position = glm::vec3(0.0f, 4.0f, -3.5f);
    djSpot.direction = glm::vec3(0.0f, -1.0f, 0.2f);
    djSpot.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    djSpot.diffuse = glm::vec3(1.0f, 1.0f, 0.8f);  // Warm spotlight
    djSpot.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    djSpot.setAngle(12.5f, 17.5f);  // Inner 12.5°, outer 17.5°
    djSpot.constant = 1.0f;
    djSpot.linear = 0.09f;
    djSpot.quadratic = 0.032f;
    spotLights.push_back(djSpot);

    //Dance floor
    // SpotLight danceSpot;
    // danceSpot.position = glm::vec3(0.0f, 5.0f, 2.0f);
    // danceSpot.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    // danceSpot.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    // danceSpot.diffuse = glm::vec3(0.8f, 0.2f, 1.0f);  // Purple
    // danceSpot.specular = glm::vec3(0.8f, 0.2f, 1.0f);
    // danceSpot.setAngle(15.0f, 20.0f);
    // danceSpot.constant = 1.0f;
    // danceSpot.linear = 0.07f;
    // danceSpot.quadratic = 0.017f;
    // spotLights.push_back(danceSpot);
}


void Scene::updateLights(float dt) {
  //Pulse for point lights
  static float time = 0.0f;
  time += dt;

  // Pulse red light
  if (pointLights.size() > 0) {
    float pulse = (sin(time * 2.0f) + 1.0f) * 0.5f;  // 0 to 1
    pointLights[0].diffuse = glm::vec3(1.0f, 0.2f, 0.2f) * (0.5f + pulse * 0.5f);
  }

  // Pulse green light (different frequency)
  if (pointLights.size() > 1) {
    float pulse = (sin(time * 2.5f) + 1.0f) * 0.5f;
    pointLights[1].diffuse = glm::vec3(0.2f, 1.0f, 0.2f) * (0.5f + pulse * 0.5f);
  }

  // Pulse blue light (different frequency)
  if (pointLights.size() > 2) {
    float pulse = (sin(time * 3.0f) + 1.0f) * 0.5f;
    pointLights[2].diffuse = glm::vec3(0.2f, 0.2f, 1.0f) * (0.5f + pulse * 0.5f);
  }

  // Rotate spotlight direction
  if (spotLights.size() > 1) {
    float angle = time * 0.5f;
    spotLights[1].direction = glm::vec3(
        sin(angle) * 0.3f,
        -1.0f,
        cos(angle) * 0.3f
    );
  }
}