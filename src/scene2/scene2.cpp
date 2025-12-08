#include <iostream>

#include <ppgso/ppgso.h>
#include <shaders/color_frag_glsl.h>
#include <shaders/color_vert_glsl.h>
#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>
#include <fstream>
#include <json.hpp>

#include "Emitter.h"
#include "scene.h"
#include "DanceFloorObject.h"
#include "DustEmitter.h"
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
    man->position = {0.0f, 0.0f, .0f};
    man->rotation = {1.55f, 0.0f, 0.0f};
    man->scale = {.01f, .01f, .01f};
    man->hasCollision = true;
    man->boxSize = glm::vec3(74.27f, 190.95f, 32.02f);
    scene.objects.push_back(man); //Man with default texture

    auto man1 = std::make_shared<Object>(man->getMesh(), "models/people/male/textures/hero_baseColor-red.png");
    man1->position = {8.0f, 0.0f, .0f};
    man1->rotation = {1.55f, 0.0f, 0.0f};
    man->boxSize = glm::vec3(37.47f, 380.95f, 16.01f); //74.47, 190.95f 32.02
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


    //loading objects for collisions
    // auto buildingInfo = scene.readMeshCollisions("models/building/scene.gltf", "Floor1_M_Floor_0");
    auto buildingInfo = building->getMesh()->getMeshBounds("Floor1_M_Floor_0");
    auto floorColider = std::make_shared<Object>(building->getMesh(), "");
    floorColider->position = building->position + (buildingInfo.center * building->scale);
    floorColider->boxSize = buildingInfo.size;
    floorColider->scale = building->scale;
    floorColider->hasCollision = true;
    floorColider->shouldRender = false;
    scene.objects.push_back(floorColider);

    auto barTop = building->getMesh()->getMeshBounds("Bar_1_M_BarTop_0");
    auto barTopCollider = std::make_shared<Object>(building->getMesh(), "");
    barTopCollider->position = building->position + (barTop.center * building->scale);
    barTopCollider->boxSize = barTop.size;
    barTopCollider->scale = building->scale;
    barTopCollider->shouldRender = false;
    barTopCollider->hasCollision = true;
    scene.objects.push_back(barTopCollider);

    auto batTop2 = building->getMesh()->getMeshBounds("Bar_2_M_BarTop_0");
    auto barTopCollider2 = std::make_shared<Object>(building->getMesh(), "");
    barTopCollider2->position = building->position + (batTop2.center * building->scale);
    barTopCollider2->boxSize = batTop2.size;
    barTopCollider2->scale = building->scale;
    barTopCollider2->shouldRender = false;
    barTopCollider2->hasCollision = true;
    scene.objects.push_back(barTopCollider2);

    //Bar_3_Short_M_BarTop_0
    auto barTopCorner = building->getMesh()->getMeshBounds("Bar_Corner_M_BarTop_0");
    auto barTopCornerCollider = std::make_shared<Object>(building->getMesh(), "");
    barTopCornerCollider->position = building->position + (barTopCorner.center * building->scale);
    barTopCornerCollider->boxSize = barTopCorner.size;
    barTopCornerCollider->scale = building->scale;
    barTopCornerCollider->shouldRender = false;
    barTopCornerCollider->hasCollision = true;
    scene.objects.push_back(barTopCornerCollider);

    auto barTopEnd = building->getMesh()->getMeshBounds("Bar_End_M_BarTop_0");
    auto barTopEndCollider = std::make_shared<Object>(building->getMesh(), "");
    barTopEndCollider->position = building->position + (barTopEnd.center * building->scale);
    barTopEndCollider->boxSize = barTopEnd.size;
    barTopEndCollider->scale = building->scale;
    barTopEndCollider->shouldRender = false;
    barTopEndCollider->hasCollision = true;
    scene.objects.push_back(barTopEndCollider);

    auto barGate = building->getMesh()->getMeshBounds("Bar_Gate_M_BarTop_0");
    auto barGateCollider = std::make_shared<Object>(building->getMesh(), "");
    barGateCollider->position = building->position + (barGate.center * building->scale);
    barGateCollider->boxSize = barGate.size;
    barGateCollider->scale = building->scale;
    barGateCollider->shouldRender = false;
    barGateCollider->hasCollision = true;
    scene.objects.push_back(barGateCollider);

    auto barShort = building->getMesh()->getMeshBounds("Bar_3_Short_M_BarTop_0");
    auto barShortCollider = std::make_shared<Object>(building->getMesh(), "");
    barShortCollider->position = building->position + (barShort.center * building->scale);
    barShortCollider->boxSize = barShort.size;
    barShortCollider->scale = building->scale;
    barShortCollider->shouldRender = false;
    barShortCollider->hasCollision = true;
    scene.objects.push_back(barShortCollider);

    // Table1_M_Table_0
    auto table1 = building->getMesh()->getMeshBounds("Table1_M_Table_0");
    auto table1Collider = std::make_shared<Object>(building->getMesh(), "");
    table1Collider->position = building->position + (table1.center * building->scale);
    table1Collider->boxSize = table1.size;
    table1Collider->scale = building->scale;
    table1Collider->shouldRender = false;
    table1Collider->hasCollision = true;
    scene.objects.push_back(table1Collider);
    // Table_M_Table_0
    auto table2 = building->getMesh()->getMeshBounds("Table_M_Table_0");
    auto table2Collider = std::make_shared<Object>(building->getMesh(), "");
    table2Collider->position = building->position + (table2.center * building->scale);
    table2Collider->boxSize = table2.size;
    table2Collider->scale = building->scale;
    table2Collider->shouldRender = false;
    table2Collider->hasCollision = true;
    scene.objects.push_back(table2Collider);
    // Steps_M_DJControlerandPainting_0
    auto djSteps = building->getMesh()->getMeshBounds("Steps_M_DJControlerandPainting_0");
    auto djStepsCollider = std::make_shared<Object>(building->getMesh(), "");
    djStepsCollider->position = building->position + (djSteps.center * building->scale);
    djStepsCollider->boxSize = djSteps.size;
    djStepsCollider->scale = building->scale;
    djStepsCollider->shouldRender = false;
    djStepsCollider->hasCollision = true;
    scene.objects.push_back(djStepsCollider);

    // Steps1_M_DJControlerandPainting_0
    // Steps2_M_DJControlerandPainting_0
    // Steps3_M_DJControlerandPainting_0
    // Steps4_M_DJControlerandPainting_0
    // Steps5_M_DJControlerandPainting_0

    // Curved_Bench_4_M_BenchEnd_0
    auto curvedBench4End = building->getMesh()->getMeshBounds("Curved_Bench_4_M_BenchEnd_0");
    auto curvedBench4EndCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench4EndCollider->position = building->position + (curvedBench4End.center * building->scale);
    curvedBench4EndCollider->boxSize = curvedBench4End.size;
    curvedBench4EndCollider->scale = building->scale;
    curvedBench4EndCollider->shouldRender = false;
    curvedBench4EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench4EndCollider);
    // Curved_Bench_4_M_BenchSeatandPicture_0
    auto curvedBench4Seat = building->getMesh()->getMeshBounds("Curved_Bench_4_M_BenchSeatandPicture_0");
    auto curvedBench4SeatCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench4SeatCollider->position = building->position + (curvedBench4Seat.center * building->scale);
    curvedBench4SeatCollider->boxSize = curvedBench4Seat.size;
    curvedBench4SeatCollider->scale = building->scale;
    curvedBench4SeatCollider->shouldRender = false;
    curvedBench4SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench4SeatCollider);
    // Curved_Bench_4_M_BenchLowerandBack_0
    auto curvedBench4Back = building->getMesh()->getMeshBounds("Curved_Bench_4_M_BenchLowerandBack_0");
    auto curvedBench4BackCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench4BackCollider->position = building->position + (curvedBench4Back.center * building->scale);
    curvedBench4BackCollider->boxSize = curvedBench4Back.size;
    curvedBench4BackCollider->scale = building->scale;
    curvedBench4BackCollider->shouldRender = false;
    curvedBench4BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench4BackCollider);

    // Curved_Bench_5_M_BenchEnd_0
    auto curvedBench5End = building->getMesh()->getMeshBounds("Curved_Bench_5_M_BenchEnd_0");
    auto curvedBench5EndCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench5EndCollider->position = building->position + (curvedBench5End.center * building->scale);
    curvedBench5EndCollider->boxSize = curvedBench5End.size;
    curvedBench5EndCollider->scale = building->scale;
    curvedBench5EndCollider->shouldRender = false;
    curvedBench5EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench5EndCollider);
    // Curved_Bench_5_M_BenchSeatandPicture_0
    auto curvedBench5Seat = building->getMesh()->getMeshBounds("Curved_Bench_5_M_BenchSeatandPicture_0");
    auto curvedBench5SeatCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench5SeatCollider->position = building->position + (curvedBench5Seat.center * building->scale);
    curvedBench5SeatCollider->boxSize = curvedBench5Seat.size;
    curvedBench5SeatCollider->scale = building->scale;
    curvedBench5SeatCollider->shouldRender = false;
    curvedBench5SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench5SeatCollider);
    // Curved_Bench_5_M_BenchLowerandBack_0
    auto curvedBench5Back = building->getMesh()->getMeshBounds("Curved_Bench_5_M_BenchLowerandBack_0");
    auto curvedBench5BackCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench5BackCollider->position = building->position + (curvedBench5Back.center * building->scale);
    curvedBench5BackCollider->boxSize = curvedBench5Back.size;
    curvedBench5BackCollider->scale = building->scale;
    curvedBench5BackCollider->shouldRender = false;
    curvedBench5BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench5BackCollider);

    // Curved_Bench_9_M_BenchEnd_0
    auto curvedBench9End = building->getMesh()->getMeshBounds("Curved_Bench_9_M_BenchEnd_0");
    auto curvedBench9EndCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench9EndCollider->position = building->position + (curvedBench9End.center * building->scale);
    curvedBench9EndCollider->boxSize = curvedBench9End.size;
    curvedBench9EndCollider->scale = building->scale;
    curvedBench9EndCollider->shouldRender = false;
    curvedBench9EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench9EndCollider);
    // Curved_Bench_9_M_BenchSeatandPicture_0
    auto curvedBench9Seat = building->getMesh()->getMeshBounds("Curved_Bench_9_M_BenchSeatandPicture_0");
    auto curvedBench9SeatCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench9SeatCollider->position = building->position + (curvedBench9Seat.center * building->scale);
    curvedBench9SeatCollider->boxSize = curvedBench9Seat.size;
    curvedBench9SeatCollider->scale = building->scale;
    curvedBench9SeatCollider->shouldRender = false;
    curvedBench9SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench9SeatCollider);
    // Curved_Bench_9_M_BenchLowerandBack_0
    auto curvedBench9Back = building->getMesh()->getMeshBounds("Curved_Bench_9_M_BenchLowerandBack_0");
    auto curvedBench9BackCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench9BackCollider->position = building->position + (curvedBench9Back.center * building->scale);
    curvedBench9BackCollider->boxSize = curvedBench9Back.size;
    curvedBench9BackCollider->scale = building->scale;
    curvedBench9BackCollider->shouldRender = false;
    curvedBench9BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench9BackCollider);

    // Curved_Bench_8_M_BenchEnd_0
    auto curvedBench8End = building->getMesh()->getMeshBounds("Curved_Bench_8_M_BenchEnd_0");
    auto curvedBench8EndCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench8EndCollider->position = building->position + (curvedBench8End.center * building->scale);
    curvedBench8EndCollider->boxSize = curvedBench8End.size;
    curvedBench8EndCollider->scale = building->scale;
    curvedBench8EndCollider->shouldRender = false;
    curvedBench8EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench8EndCollider);
    // Curved_Bench_8_M_BenchSeatandPicture_0
    auto curvedBench8Seat = building->getMesh()->getMeshBounds("Curved_Bench_8_M_BenchSeatandPicture_0");
    auto curvedBench8SeatCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench8SeatCollider->position = building->position + (curvedBench8Seat.center * building->scale);
    curvedBench8SeatCollider->boxSize = curvedBench8Seat.size;
    curvedBench8SeatCollider->scale = building->scale;
    curvedBench8SeatCollider->shouldRender = false;
    curvedBench8SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench8SeatCollider);
    // Curved_Bench_8_M_BenchLowerandBack_0
    auto curvedBench8Back = building->getMesh()->getMeshBounds("Curved_Bench_8_M_BenchLowerandBack_0");
    auto curvedBench8BackCollider = std::make_shared<Object>(building->getMesh(), "");
    curvedBench8BackCollider->position = building->position + (curvedBench8Back.center * building->scale);
    curvedBench8BackCollider->boxSize = curvedBench8Back.size;
    curvedBench8BackCollider->scale = building->scale;
    curvedBench8BackCollider->shouldRender = false;
    curvedBench8BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench8BackCollider);

    // Large_Speaker_M_LargeSpeakerandGlasses_0
    auto largeSpeaker = building->getMesh()->getMeshBounds("Large_Speaker_M_LargeSpeakerandGlasses_0");
    auto largeSpeakerCollider = std::make_shared<Object>(building->getMesh(), "");
    largeSpeakerCollider->position = building->position + (largeSpeaker.center * building->scale);
    largeSpeakerCollider->boxSize = largeSpeaker.size;
    largeSpeakerCollider->scale = building->scale;
    largeSpeakerCollider->shouldRender = false;
    largeSpeakerCollider->hasCollision = true;
    scene.objects.push_back(largeSpeakerCollider);
    // Large_Speaker1_M_LargeSpeakerandGlasses_0
    auto largeSpeaker1 = building->getMesh()->getMeshBounds("Large_Speaker1_M_LargeSpeakerandGlasses_0");
    auto largeSpeaker1Collider = std::make_shared<Object>(building->getMesh(), "");
    largeSpeaker1Collider->position = building->position + (largeSpeaker1.center * building->scale);
    largeSpeaker1Collider->boxSize = largeSpeaker1.size;
    largeSpeaker1Collider->scale = building->scale;
    largeSpeaker1Collider->shouldRender = false;
    largeSpeaker1Collider->hasCollision = true;
    scene.objects.push_back(largeSpeaker1Collider);

    // DJ_Stand_M_DJStandLower_0
    auto djStand = building->getMesh()->getMeshBounds("DJ_Stand_M_DJStandLower_0");
    auto djStandCollider = std::make_shared<Object>(building->getMesh(), "");
    djStandCollider->position = building->position + (djStand.center * building->scale);
    djStandCollider->boxSize = djStand.size;
    djStandCollider->scale = building->scale;
    djStandCollider->shouldRender = false;
    djStandCollider->hasCollision = true;
    scene.objects.push_back(djStandCollider);
    // DJ_Stand_M_DJstandUpper_0
    auto djStandUpper = building->getMesh()->getMeshBounds("DJ_Stand_M_DJstandUpper_0");
    auto djStandUpperCollider = std::make_shared<Object>(building->getMesh(), "");
    djStandUpperCollider->position = building->position + (djStandUpper.center * building->scale);
    djStandUpperCollider->boxSize = djStandUpper.size;
    djStandUpperCollider->scale = building->scale;
    djStandUpperCollider->shouldRender = false;
    djStandUpperCollider->hasCollision = true;
    scene.objects.push_back(djStandUpperCollider);

    auto whiskeyGlass = std::make_shared<WhiskeyGlassObject>("models/whiskey_glass/scene.gltf");
    whiskeyGlass->position = {-3.1f, .95f, 0.0f};
    whiskeyGlass->rotation = {0.0f, 0.0f, 0.0f};
    whiskeyGlass->scale = {.6f, .6f, .6f};

    whiskeyGlass->setTransparentMeshes({"pCube1"});
    whiskeyGlass->transparency = .4f;
    scene.objects.push_back(whiskeyGlass); //Whiskey glass

    //smokeCloud effect without collision or physics===========================
    auto quadMesh = std::make_shared<ppgso::Mesh_Assimp>("models/smoke_cloud/plane.obj");
    auto smokeCloud = std::make_shared<Emitter>(quadMesh, "models/smoke_cloud/smoke_cloud_transparent.png");
    smokeCloud->position = {0.0f, 0.0f, 0.0f};
    smokeCloud->spawnPosition = {0.0f, 0.2f, 0.0f};
    scene.objects.push_back(std::move(smokeCloud));

    auto quadMesh2 = std::make_shared<ppgso::Mesh_Assimp>("models/smoke_cloud/plane.obj");
    auto smokeCloud2 = std::make_shared<Emitter>(quadMesh2, "models/smoke_cloud/smoke_cloud_transparent.png");
    smokeCloud2->position = {0.0f, 0.0f, 0.0f};
    smokeCloud2->spawnPosition = {0.f, 0.2f, 2.0f};
    scene.objects.push_back(std::move(smokeCloud2));

    //===============dust particles======================
    auto quadMeshDust = std::make_shared<ppgso::Mesh_Assimp>("models/dust_particle/dustParticle.obj");
    auto dust = std::make_shared<DustEmitter>(quadMeshDust, "models/dust_particle/dustParticle.png");
    dust->position = {0.0f, 4.0f, 0.0f};
    scene.objects.push_back(std::move(dust));

    auto quadMeshDust2 = std::make_shared<ppgso::Mesh_Assimp>("models/dust_particle/dustParticle.obj");
    auto dust2 = std::make_shared<DustEmitter>(quadMeshDust2, "models/dust_particle/dustParticle.png");
    dust2->position = {-3.f, 2.4f, 0.0f};
    dust2->spawnAreaMax = glm::vec3(-1.0f, 0.0f, -5.0f);
    dust2->spawnAreaMin = glm::vec3(1.0f, 0.0f, 5.0f);
    scene.objects.push_back(std::move(dust2));

    auto quadMeshDust3 = std::make_shared<ppgso::Mesh_Assimp>("models/dust_particle/dustParticle.obj");
    auto dust3 = std::make_shared<DustEmitter>(quadMeshDust3, "models/dust_particle/dustParticle.png");
    dust3->position = {3.f, 2.4f, 0.0f};
    dust3->spawnAreaMax = glm::vec3(-1.0f, 0.0f, -5.0f);
    dust3->spawnAreaMin = glm::vec3(1.0f, 0.0f, 5.0f);
    scene.objects.push_back(std::move(dust3));
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