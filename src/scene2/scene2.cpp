#include <iostream>
// #include <filesystem>

#include <ppgso/ppgso.h>
#include <shaders/color_frag_glsl.h>
#include <shaders/color_vert_glsl.h>
#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>

#include "scene.h"

const unsigned int SIZE = 960;

//Main window
class SceneWindow : public ppgso::Window {
private:

  //Debug tools
  bool showGrid = true;
  bool freeCamera = false;

  Scene scene;

  // Time tracking
  float time = 0.0f;

  // Mouse tracking
  double lastMouseX = SIZE / 2.0;
  double lastMouseY = SIZE / 2.0;
  bool firstMouse = true;

  //Loading all models
  void loadObjects() {

    // std::cout << "Current working directory: "
    //         << std::filesystem::current_path() << std::endl;

    auto building = std::make_shared<Object>("../src/scene2/models/building/scene.gltf");
    building->position = {0.0f, 0.0f, 0.0f};
    building->rotation = {0.0f, 0.0f, 0.0f};
    building->scale = {.01f, .01f, .01f};
    scene.objects.push_back(building); //Main Building

    auto man = std::make_shared<Object>("../src/scene2/models/people/male/scene.gltf");
    man->position = {7.0f, 0.0f, .0f};
    man->rotation = {0.0f, 0.0f, 0.0f};
    man->scale = {.01f, .01f, .01f};
    scene.objects.push_back(man); //Man with default texture

    auto man1 = std::make_shared<Object>(man->getMesh(), "../src/scene2/models/people/male/textures/hero_baseColor-red.png");
    man1->position = {8.0f, 0.0f, .0f};
    man1->rotation = {0.0f, 0.0f, 0.0f};
    man1->scale = {.01f, .01f, .01f};
    scene.objects.push_back(man1); //Man with different texture

    // auto woman = std::make_shared<Object>("D:\\C#\\PPGSO\\ppgso-public\\src\\scene2\\models\\people\\female\\scene.gltf");
    // woman->position = {9.0f, 0.0f, .0f};
    // woman->rotation = {0.0f, 0.0f, 0.0f};
    // woman->scale = {.01f, .01f, .01f};
    // scene.objects.push_back(woman); //Woman

    auto danceFloor = std::make_shared<Object>("../src/scene2/models/dance_floor/scene.gltf");
    danceFloor->position = {0.0f, -0.05f, 0.15f};
    danceFloor->rotation = {0.0f, 0.0f, 0.0f};
    danceFloor->scale = {.185f, .185f, .185f};
    scene.objects.push_back(danceFloor); //Dance floor

    auto discoBall = std::make_shared<Object>("../src/scene2/models/disco_ball/scene.gltf");
    discoBall->position = {0.0f, 5.5f, 0.0f};
    discoBall->rotation = {0.0f, 0.0f, 0.0f};
    discoBall->scale = {1.5f, 1.5f, 1.5f};
    scene.objects.push_back(discoBall); //Disco ball
  }

  //Create scene
  void initScene() {
    scene.objects.clear();

    // Create a camera
    auto camera = std::make_unique<Camera>(60.0f, 1.0f, 0.1f, 100.0f);
    camera->position.y = 1.f;  //Camera position
    scene.camera = move(camera);

    loadObjects();
  }

  static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    SceneWindow* win = static_cast<SceneWindow*>(glfwGetWindowUserPointer(window));
    if (win) {
      win->handleMouseMove(xpos, ypos);
    }
  }

  void handleMouseMove(double xpos, double ypos) {
    // Only handle mouse movement if free camera is enabled
    if (!freeCamera) return;

    if (firstMouse) {
      lastMouseX = xpos;
      lastMouseY = ypos;
      firstMouse = false;
      return;
    }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;

    lastMouseX = xpos;
    lastMouseY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    scene.camera->yaw += xoffset;
    scene.camera->pitch += yoffset;

    if (scene.camera->pitch > 89.0f)
      scene.camera->pitch = 89.0f;
    if (scene.camera->pitch < -89.0f)
      scene.camera->pitch = -89.0f;

    scene.camera->updateRotation();
  }

public:
  SceneWindow() : Window{"Scene2", SIZE, SIZE} {
    //hideCursor();
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    initScene();

    time = (float)glfwGetTime();
  }

  void onKey(int key, int scanCode, int action, int mods) override {
    scene.keyboard[key] = action;

    // Toggle debug features
    if (action == GLFW_PRESS) {
      if (key == GLFW_KEY_G) {
        showGrid = !showGrid;
        std::cout << "Grid: " << (showGrid ? "ON" : "OFF") << std::endl;
      }
      if (key == GLFW_KEY_C) {
        freeCamera = !freeCamera;
        std::cout << "Free Camera: " << (freeCamera ? "ON" : "OFF") << std::endl;

        if (freeCamera) {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          firstMouse = true;
        } else {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
      }

      // Reset camera position
      if (key == GLFW_KEY_R) {
        scene.camera->position = glm::vec3(0, 0, 0);
        scene.camera->yaw = 0.0f;
        scene.camera->pitch = 0.0f;
        scene.camera->updateRotation();
        std::cout << "Camera reset" << std::endl;
      }

      // Print camera position for debugging
      if (key == GLFW_KEY_P) {
        std::cout << "=== Camera Debug Info ===" << std::endl;
        std::cout << "Position: ("
                  << scene.camera->position.x << ", "
                  << scene.camera->position.y << ", "
                  << scene.camera->position.z << ")" << std::endl;
        std::cout << "Rotation: Yaw=" << scene.camera->yaw
                  << " Pitch=" << scene.camera->pitch << std::endl;
        std::cout << "Looking direction: ("
                  << -scene.camera->back.x << ", "
                  << -scene.camera->back.y << ", "
                  << -scene.camera->back.z << ")" << std::endl;
        std::cout << "=======================" << std::endl;
      }
    }
  }

  void onIdle() override {
    // Track time
    float currentTime = (float)glfwGetTime();
    float dt = currentTime - time;
    time = currentTime;

    glClearColor(.2f, .2f, .2f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.update(dt);
    scene.render();

    // Render grid if enabled
    if (showGrid) {
      // static int frameCount = 0;
      // if (frameCount++ % 120 == 0) {
      //   std::cout << "Grid rendering active at camera: ("
      //             << scene.camera->position.x << ", "
      //             << scene.camera->position.y << ", "
      //             << scene.camera->position.z << ")" << std::endl;
      // }
      scene.renderGrid();
    }
  }
};

int main() {
  std::cout << "=== Controls ===" << std::endl;
  std::cout << "WASD - Move camera" << std::endl;
  std::cout << "Q/E - Move down/up" << std::endl;
  std::cout << "Arrow Keys - Rotate camera (keyboard mode)" << std::endl;
  std::cout << "Mouse - Look around (when free camera enabled)" << std::endl;
  std::cout << "G - Toggle grid" << std::endl;
  std::cout << "C - Toggle free camera (mouse look)" << std::endl;
  std::cout << "R - Reset camera position" << std::endl;
  std::cout << "P - Print camera position and rotation" << std::endl;
  std::cout << "=================" << std::endl;

  SceneWindow window;

  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}