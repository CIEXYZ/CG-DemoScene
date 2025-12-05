#include <iostream>

#include <ppgso/ppgso.h>
#include <shaders/color_frag_glsl.h>
#include <shaders/color_vert_glsl.h>
#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>

#include "Emitter.h"
#include "scene.h"
#include "DanceFloorObject.h"
#include "WhiskeyGlassObject.h"
#include "PersonObject.h"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

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
  double lastMouseX = WIDTH / 2.0;
  double lastMouseY = HEIGHT / 2.0;
  bool firstMouse = true;

  // Window size tracking
  int currentWidth = WIDTH;
  int currentHeight = HEIGHT;

  //Loading all models
  void loadObjects() {
    auto man = std::make_shared<Object>("models/people/male/scene.gltf");
    man->position = {7.0f, 0.0f, .0f};
    man->rotation = {1.55f, 0.0f, 0.0f};
    man->scale = {.01f, .01f, .01f};
    scene.objects.push_back(man); //Man with default texture

    auto man1 = std::make_shared<Object>(man->getMesh(), "models/people/male/textures/hero_baseColor-red.png");
    man1->position = {8.0f, 0.0f, .0f};
    man1->rotation = {1.55f, 0.0f, 0.0f};
    man1->scale = {.01f, .01f, .01f};
    scene.objects.push_back(man1); //Man with different texture

    auto woman = std::make_shared<PersonObject>("models/people/female/scene.gltf", "models/people/female/animations/idle_animation.fbx");
    woman->position = {9.0f, 0.0f, 0.0f};
    woman->rotation = {0.0f, 0.0f, 0.0f};
    woman->scale = {.011f, .011f, .011f};
    scene.objects.push_back(woman); //Woman

    auto danceFloor = std::make_shared<DanceFloorObject>();
    danceFloor->position = {0.0f, -0.05f, 0.0f};
    danceFloor->rotation = {0.0f, 0.0f, 0.0f};
    danceFloor->scale = {0.3f, 0.3f, 0.3f};
    scene.objects.push_back(danceFloor); //Dance floor

    auto discoBall = std::make_shared<Object>("models/disco_ball/scene.gltf"); //TODO: fix texture
    discoBall->position = {0.0f, 4.6f, 0.0f};
    discoBall->rotation = {0.0f, 0.0f, 0.0f};
    discoBall->scale = {1.2f, 1.2f, 1.2f};
    scene.objects.push_back(discoBall); //Disco ball

    auto building = std::make_shared<Object>("models/building/scene.gltf");
    building->position = {0.0f, 0.0f, 0.0f};
    building->rotation = {0.0f, 0.0f, 0.0f};
    building->scale = {1.0f, 1.0f, 1.0f};

    building->setTransparentMeshes({"Glass", "Shot_Glass", "WineGlass",
                                              "BeerBottle", "Large_Bottle", "Rectangle_Bottle",
                                              "Sphere_Bottle", "Tall_Skinny_Bottle"});
    building->transparency = 3.f;

    scene.objects.push_back(building); //Main Building

    auto whiskeyGlass = std::make_shared<WhiskeyGlassObject>("models/whiskey_glass/scene.gltf");
    whiskeyGlass->position = {-3.1f, .95f, 0.0f};
    whiskeyGlass->rotation = {0.0f, 0.0f, 0.0f};
    whiskeyGlass->scale = {.6f, .6f, .6f};

    whiskeyGlass->setTransparentMeshes({"pCube1"});
    whiskeyGlass->transparency = .4f;
    scene.objects.push_back(whiskeyGlass); //Whiskey glass

    auto quadMesh = std::make_shared<ppgso::Mesh_Assimp>("models/smoke_cloud/plane.obj");
    auto smokeCloud = std::make_shared<Emitter>(quadMesh, "models/smoke_cloud/smoke_cloud_transparent.png");
    smokeCloud->position = {0.0f, 0.0f, 0.0f};
    smokeCloud->spawnPosition = {0.0f, 0.2f, 0.0f};
    smokeCloud->spawnRate = 1.f;
    scene.objects.push_back(std::move(smokeCloud));

    auto quadMesh2 = std::make_shared<ppgso::Mesh_Assimp>("models/smoke_cloud/plane.obj");
    auto smokeCloud2 = std::make_shared<Emitter>(quadMesh2, "models/smoke_cloud/smoke_cloud_transparent.png");
    smokeCloud2->position = {0.0f, 0.0f, 0.0f};
    smokeCloud2->spawnPosition = {0.f, 0.2f, 2.0f};
    smokeCloud2->spawnRate = 1.f;
    scene.objects.push_back(std::move(smokeCloud2));
  }

  //Create scene
  void initScene() {
    scene.objects.clear();

    // Create a camera
    auto camera = std::make_unique<Camera>(60.0f, (float)currentWidth / (float)currentHeight, 0.1f, 100.0f);
    camera->position.y = 1.f;  //Camera position
    scene.camera = move(camera);

    loadObjects();
  }

  static void windowSizeCallback(GLFWwindow* window, int width, int height) {
    SceneWindow* win = static_cast<SceneWindow*>(glfwGetWindowUserPointer(window));
    if (win) {
      win->handleWindowResize(width, height);
    }
  }

  void handleWindowResize(int width, int height) {
    currentWidth = width;
    currentHeight = height;

    // Update viewport
    glViewport(0, 0, width, height);

    // Update camera aspect ratio
    if (scene.camera) {
      scene.camera->projectionMatrix = glm::perspective(
        glm::radians(60.0f),
        (float)width / (float)height,
        0.1f,
        100.0f
      );
    }
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
  SceneWindow() : Window{"Scene2", WIDTH, HEIGHT} {
    //hideCursor();
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glViewport(0, 0, WIDTH, HEIGHT);

    initScene();

    handleWindowResize(WIDTH, HEIGHT);

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

      if (key == GLFW_KEY_F) {
        // Toggle fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (glfwGetWindowMonitor(window) == nullptr) {
          // Go fullscreen
          glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
          std::cout << "Fullscreen mode" << std::endl;
        } else {
          // Go windowed
          glfwSetWindowMonitor(window, nullptr, 100, 100, WIDTH, HEIGHT, 0);
          std::cout << "Windowed mode" << std::endl;
        }
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
  std::cout << "F - Toggle fullscreen" << std::endl;
  std::cout << "=================" << std::endl;

  SceneWindow window;

  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}