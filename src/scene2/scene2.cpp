#include <iostream>

#include <ppgso/ppgso.h>
#include <shaders/color_frag_glsl.h>
#include <shaders/color_vert_glsl.h>
#include <shaders/diffuse_vert_glsl.h>
#include <shaders/diffuse_frag_glsl.h>
#include <fstream>

#include "Emitter.h"
#include "scene.h"
#include "DanceFloorObject.h"
#include "DustEmitter.h"
#include "WhiskeyGlassObject.h"
#include "PersonObject.h"
#include "Timeline.h"
#include "Crowd.h"
#include "Skybox.h"
#include "PostProcessing.h"
#include "ShadowMap.h"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

//Main window
class SceneWindow : public ppgso::Window {
private:

  //Debug tools
  bool showGrid = true;
  bool freeCamera = false;

  Scene scene;

  // Post-processing
  std::unique_ptr<PostProcessing> postProcessing;

  // Time tracking
  float time = 0.0f;

  // Mouse tracking
  double lastMouseX = WIDTH / 2.0;
  double lastMouseY = HEIGHT / 2.0;
  bool firstMouse = true;

  // Window size tracking
  int currentWidth = WIDTH;
  int currentHeight = HEIGHT;
  int windowedWidth = WIDTH;
  int windowedHeight = HEIGHT;
  int windowedPosX = 100;
  int windowedPosY = 100;

  //Loading all models
  void loadObjects() {
    //People
    auto man = std::make_shared<PersonObject>("models/people/male/scene.gltf", "models/people/male/animations/sit_animation.fbx");
    man->loadAnimation("models/people/male/animations/standup_animation.fbx");
    man->loadAnimation("models/people/male/animations/hit_shoulder_animation.fbx");
    man->name = "man";
    man->position = {-0.1f, 0.0f, -0.1f};
    man->rotation = {0.0f, -5.0f, 0.0f};
    man->scale = {.008f, .008f, .008f};
    man->hasCollision = true;
    man->boxSize = glm::vec3(74.27f, 190.95f, 32.02f);
    scene.objects.push_back(man); //Main man

    auto man1 = std::make_shared<PersonObject>("models/people/male/scene.gltf", "models/people/male/animations/sit1_animation.fbx");
    man1->loadAnimation("models/people/male/animations/standup_animation.fbx");
    man1->name = "man1";
    man1->position = {7.0f, 0.0f, .0f};
    man1->rotation = {0.0f, 0.0f, 0.0f};
    man1->boxSize = glm::vec3(37.47f, 380.95f, 16.01f);
    man1->scale = {.008f, .008f, .008f};
    scene.objects.push_back(man1); //Man 1

    auto man2 = std::make_shared<PersonObject>("models/people/male/scene.gltf", "models/people/male/animations/sit_animation.fbx");
    man2->loadAnimation("models/people/male/animations/standup_animation.fbx");
    man2->name = "man2";
    man2->position = {7.0f, 0.0f, .0f};
    man2->rotation = {0.0f, 0.0f, 0.0f};
    man2->scale = {.008f, .008f, .008f};
    scene.objects.push_back(man2); //Man 2

    auto woman = std::make_shared<PersonObject>("models/people/female/scene.gltf", "models/people/female/animations/sit_animation.fbx");
    woman->loadAnimation("models/people/female/animations/standup_animation.fbx");
    woman->name = "woman";
    woman->position = {3.0f, 0.0f, 0.0f};
    woman->rotation = {0.0f, 0.0f, 0.0f};
    woman->scale = {.009f, .009f, .009f};
    scene.objects.push_back(woman); //Woman

    auto madMan = std::make_shared<PersonObject>("models/people/male/scene.gltf", "models/people/male/animations/punch_animation.fbx");
    madMan->name = "madMan";
    madMan->position = {0.4f, 0.0f, -0.1f};
    madMan->rotation = {0.0f, 5.0f, 0.0f};
    madMan->scale = {.008f, .008f, .008f};
    scene.objects.push_back(madMan); //Mad man

    auto barman = std::make_shared<PersonObject>("models/people/male/scene.gltf", "models/people/male/animations/bartending_animation.fbx");
    barman->loadAnimation("models/people/male/animations/putdown_animation.fbx");
    barman->name = "barman";
    barman->position = {-3.9f, 0.0f, 0.0f};
    barman->rotation = {-1.57f, 0.0f, -4.8f};
    barman->scale = {.008f, .008f, .008f};
    scene.objects.push_back(barman); //Barman

    auto manNpc = std::make_shared<PersonObject>("models/people/male/scene.gltf", "models/people/male/animations/idle1_animation.fbx");
    manNpc->loadAnimation("models/people/male/animations/idle_animation.fbx");
    manNpc->name = "manNpc";
    manNpc->position = {9.0f, 0.0f, -3.0f};
    manNpc->rotation = {0.0f, 0.0f, 0.0f};
    manNpc->scale = {.008f, .008f, .008f};
    scene.objects.push_back(manNpc); //Man nps

    auto womanNpc = std::make_shared<PersonObject>("models/people/female/scene.gltf", "models/people/female/animations/dance_animation.fbx");
    womanNpc->loadAnimation("models/people/female/animations/idle_animation.fbx");
    womanNpc->loadAnimation("models/people/female/animations/idle1_animation.fbx");
    womanNpc->name = "womanNpc";
    womanNpc->position = {10.0f, 0.0f, -3.0f};
    womanNpc->rotation = {0.0f, 0.0f, 0.0f};
    womanNpc->scale = {.009f, .009f, .009f};
    scene.objects.push_back(womanNpc); //Woman npc

    auto crowd = std::make_shared<Crowd>();

    crowd->addPersonType(manNpc, {
        "models/people/male/textures/hero_baseColor.png",
        "models/people/male/textures/hero_baseColor-red.png"
    });

    crowd->addPersonType(womanNpc, {
        "models/people/female/textures/secretary_mat_baseColor.png"
    });

    //Generate crowd with gap in center (for line formation)
    PoissonDiskSampler::Rect bounds(-2, -2, 3.8, 6);
    PoissonDiskSampler::Rect gap(0, 1.5, 4, 0.8);  // Small line gap in center
    crowd->generateCrowd(bounds, 0.5f, glm::vec3(0,0,0), glm::vec3(0.008f), &gap);

    //Example with 5000+ instances
    // PoissonDiskSampler::Rect bounds(-25, -25, 26, 26);
    // PoissonDiskSampler::Rect gap(0, 0, 0, 0);  // Small line gap in center
    // crowd->generateCrowd(bounds, 0.2f, glm::vec3(0,0,0), glm::vec3(0.008f), &gap);

    scene.objects.push_back(crowd);

    //Other objects
    auto danceFloor = std::make_shared<DanceFloorObject>();
    danceFloor->position = {0.0f, -0.05f, 0.0f};
    danceFloor->rotation = {0.0f, 0.0f, 0.0f};
    danceFloor->scale = {0.3f, 0.3f, 0.3f};
    scene.objects.push_back(danceFloor); //Dance floor

    auto discoBall = std::make_shared<Object>("models/disco_ball/scene.gltf"); //TODO: fix texture
    discoBall->position = {0.0f, 5.0f, 1.0f};
    discoBall->rotation = {0.0f, 0.0f, 0.0f};
    discoBall->scale = {1.2f, 1.2f, 1.2f};
    scene.objects.push_back(discoBall); //Disco ball

    scene.building = std::make_shared<Object>("models/building/scene.gltf");
    scene.building->useNormalMapping = true;
    scene.building->setNormalMap(17, "models/building/textures/M_Floor_normal.png");
    scene.building->setNormalMap(13, "models/building/textures/M_BenchSeatandPicture_normal.png");
    scene.building->setNormalMap(1, "models/building/textures/M_LargeSpeakerandGlasses_normal.png");
    scene.building->setNormalMap(5, "models/building/textures/M_LightsMediumProps_normal.png");
    scene.building->setNormalMap(9, "models/building/textures/M_Table_normal.png");
    scene.building->position = {0.0f, 0.0f, 0.0f};
    scene.building->rotation = {0.0f, 0.0f, 0.0f};
    scene.building->scale = {1.0f, 1.0f, 1.0f};

    scene.building->setTransparentMeshes({"Glass", "Shot_Glass", "WineGlass",
                                              "BeerBottle", "Large_Bottle", "Rectangle_Bottle",
                                              "Sphere_Bottle", "Tall_Skinny_Bottle"});
    scene.building->transparency = 3.f;

    scene.objects.push_back(scene.building); //Main Building

    //loading objects for collisions
    // auto buildingInfo = scene.readMeshCollisions("models/building/scene.gltf", "Floor1_M_Floor_0");
    auto buildingInfo = scene.building->getMesh()->getMeshBounds("Floor1_M_Floor_0");
    auto floorColider = std::make_shared<Object>(scene.building->getMesh(), "");
    floorColider->position = scene.building->position + (buildingInfo.center * scene.building->scale);
    floorColider->boxSize = buildingInfo.size;
    floorColider->scale = scene.building->scale;
    floorColider->hasCollision = true;
    floorColider->shouldRender = false;
    scene.objects.push_back(floorColider);

    auto barTop = scene.building->getMesh()->getMeshBounds("Bar_1_M_BarTop_0");
    auto barTopCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    barTopCollider->position = scene.building->position + (barTop.center * scene.building->scale);
    barTopCollider->boxSize = barTop.size;
    barTopCollider->scale = scene.building->scale;
    barTopCollider->shouldRender = false;
    barTopCollider->hasCollision = true;
    scene.objects.push_back(barTopCollider);

    auto batTop2 = scene.building->getMesh()->getMeshBounds("Bar_2_M_BarTop_0");
    auto barTopCollider2 = std::make_shared<Object>(scene.building->getMesh(), "");
    barTopCollider2->position = scene.building->position + (batTop2.center * scene.building->scale);
    barTopCollider2->boxSize = batTop2.size;
    barTopCollider2->scale = scene.building->scale;
    barTopCollider2->shouldRender = false;
    barTopCollider2->hasCollision = true;
    scene.objects.push_back(barTopCollider2);

    //Bar_3_Short_M_BarTop_0
    auto barTopCorner = scene.building->getMesh()->getMeshBounds("Bar_Corner_M_BarTop_0");
    auto barTopCornerCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    barTopCornerCollider->position = scene.building->position + (barTopCorner.center * scene.building->scale);
    barTopCornerCollider->boxSize = barTopCorner.size;
    barTopCornerCollider->scale = scene.building->scale;
    barTopCornerCollider->shouldRender = false;
    barTopCornerCollider->hasCollision = true;
    scene.objects.push_back(barTopCornerCollider);

    auto barTopEnd = scene.building->getMesh()->getMeshBounds("Bar_End_M_BarTop_0");
    auto barTopEndCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    barTopEndCollider->position = scene.building->position + (barTopEnd.center * scene.building->scale);
    barTopEndCollider->boxSize = barTopEnd.size;
    barTopEndCollider->scale = scene.building->scale;
    barTopEndCollider->shouldRender = false;
    barTopEndCollider->hasCollision = true;
    scene.objects.push_back(barTopEndCollider);

    auto barGate = scene.building->getMesh()->getMeshBounds("Bar_Gate_M_BarTop_0");
    auto barGateCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    barGateCollider->position = scene.building->position + (barGate.center * scene.building->scale);
    barGateCollider->boxSize = barGate.size;
    barGateCollider->scale = scene.building->scale;
    barGateCollider->shouldRender = false;
    barGateCollider->hasCollision = true;
    scene.objects.push_back(barGateCollider);

    auto barShort = scene.building->getMesh()->getMeshBounds("Bar_3_Short_M_BarTop_0");
    auto barShortCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    barShortCollider->position = scene.building->position + (barShort.center * scene.building->scale);
    barShortCollider->boxSize = barShort.size;
    barShortCollider->scale = scene.building->scale;
    barShortCollider->shouldRender = false;
    barShortCollider->hasCollision = true;
    scene.objects.push_back(barShortCollider);

    // Table1_M_Table_0
    auto table1 = scene.building->getMesh()->getMeshBounds("Table1_M_Table_0");
    auto table1Collider = std::make_shared<Object>(scene.building->getMesh(), "");
    table1Collider->position = scene.building->position + (table1.center * scene.building->scale);
    table1Collider->boxSize = table1.size;
    table1Collider->scale = scene.building->scale;
    table1Collider->shouldRender = false;
    table1Collider->hasCollision = true;
    scene.objects.push_back(table1Collider);
    // Table_M_Table_0
    auto table2 = scene.building->getMesh()->getMeshBounds("Table_M_Table_0");
    auto table2Collider = std::make_shared<Object>(scene.building->getMesh(), "");
    table2Collider->position = scene.building->position + (table2.center * scene.building->scale);
    table2Collider->boxSize = table2.size;
    table2Collider->scale = scene.building->scale;
    table2Collider->shouldRender = false;
    table2Collider->hasCollision = true;
    scene.objects.push_back(table2Collider);
    // Steps_M_DJControlerandPainting_0
    auto djSteps = scene.building->getMesh()->getMeshBounds("Steps_M_DJControlerandPainting_0");
    auto djStepsCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    djStepsCollider->position = scene.building->position + (djSteps.center * scene.building->scale);
    djStepsCollider->boxSize = djSteps.size;
    djStepsCollider->scale = scene.building->scale;
    djStepsCollider->shouldRender = false;
    djStepsCollider->hasCollision = true;
    scene.objects.push_back(djStepsCollider);

    // Steps1_M_DJControlerandPainting_0
    // Steps2_M_DJControlerandPainting_0
    // Steps3_M_DJControlerandPainting_0
    // Steps4_M_DJControlerandPainting_0
    // Steps5_M_DJControlerandPainting_0

    // Curved_Bench_4_M_BenchEnd_0
    auto curvedBench4End = scene.building->getMesh()->getMeshBounds("Curved_Bench_4_M_BenchEnd_0");
    auto curvedBench4EndCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench4EndCollider->position = scene.building->position + (curvedBench4End.center * scene.building->scale);
    curvedBench4EndCollider->boxSize = curvedBench4End.size;
    curvedBench4EndCollider->scale = scene.building->scale;
    curvedBench4EndCollider->shouldRender = false;
    curvedBench4EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench4EndCollider);
    // Curved_Bench_4_M_BenchSeatandPicture_0
    auto curvedBench4Seat = scene.building->getMesh()->getMeshBounds("Curved_Bench_4_M_BenchSeatandPicture_0");
    auto curvedBench4SeatCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench4SeatCollider->position = scene.building->position + (curvedBench4Seat.center * scene.building->scale);
    curvedBench4SeatCollider->boxSize = curvedBench4Seat.size;
    curvedBench4SeatCollider->scale = scene.building->scale;
    curvedBench4SeatCollider->shouldRender = false;
    curvedBench4SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench4SeatCollider);
    // Curved_Bench_4_M_BenchLowerandBack_0
    auto curvedBench4Back = scene.building->getMesh()->getMeshBounds("Curved_Bench_4_M_BenchLowerandBack_0");
    auto curvedBench4BackCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench4BackCollider->position = scene.building->position + (curvedBench4Back.center * scene.building->scale);
    curvedBench4BackCollider->boxSize = curvedBench4Back.size;
    curvedBench4BackCollider->scale = scene.building->scale;
    curvedBench4BackCollider->shouldRender = false;
    curvedBench4BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench4BackCollider);

    // Curved_Bench_5_M_BenchEnd_0
    auto curvedBench5End = scene.building->getMesh()->getMeshBounds("Curved_Bench_5_M_BenchEnd_0");
    auto curvedBench5EndCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench5EndCollider->position = scene.building->position + (curvedBench5End.center * scene.building->scale);
    curvedBench5EndCollider->boxSize = curvedBench5End.size;
    curvedBench5EndCollider->scale = scene.building->scale;
    curvedBench5EndCollider->shouldRender = false;
    curvedBench5EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench5EndCollider);
    // Curved_Bench_5_M_BenchSeatandPicture_0
    auto curvedBench5Seat = scene.building->getMesh()->getMeshBounds("Curved_Bench_5_M_BenchSeatandPicture_0");
    auto curvedBench5SeatCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench5SeatCollider->position = scene.building->position + (curvedBench5Seat.center * scene.building->scale);
    curvedBench5SeatCollider->boxSize = curvedBench5Seat.size;
    curvedBench5SeatCollider->scale = scene.building->scale;
    curvedBench5SeatCollider->shouldRender = false;
    curvedBench5SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench5SeatCollider);
    // Curved_Bench_5_M_BenchLowerandBack_0
    auto curvedBench5Back = scene.building->getMesh()->getMeshBounds("Curved_Bench_5_M_BenchLowerandBack_0");
    auto curvedBench5BackCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench5BackCollider->position = scene.building->position + (curvedBench5Back.center * scene.building->scale);
    curvedBench5BackCollider->boxSize = curvedBench5Back.size;
    curvedBench5BackCollider->scale = scene.building->scale;
    curvedBench5BackCollider->shouldRender = false;
    curvedBench5BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench5BackCollider);

    // Curved_Bench_9_M_BenchEnd_0
    auto curvedBench9End = scene.building->getMesh()->getMeshBounds("Curved_Bench_9_M_BenchEnd_0");
    auto curvedBench9EndCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench9EndCollider->position = scene.building->position + (curvedBench9End.center * scene.building->scale);
    curvedBench9EndCollider->boxSize = curvedBench9End.size;
    curvedBench9EndCollider->scale = scene.building->scale;
    curvedBench9EndCollider->shouldRender = false;
    curvedBench9EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench9EndCollider);
    // Curved_Bench_9_M_BenchSeatandPicture_0
    auto curvedBench9Seat = scene.building->getMesh()->getMeshBounds("Curved_Bench_9_M_BenchSeatandPicture_0");
    auto curvedBench9SeatCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench9SeatCollider->position = scene.building->position + (curvedBench9Seat.center * scene.building->scale);
    curvedBench9SeatCollider->boxSize = curvedBench9Seat.size;
    curvedBench9SeatCollider->scale = scene.building->scale;
    curvedBench9SeatCollider->shouldRender = false;
    curvedBench9SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench9SeatCollider);
    // Curved_Bench_9_M_BenchLowerandBack_0
    auto curvedBench9Back = scene.building->getMesh()->getMeshBounds("Curved_Bench_9_M_BenchLowerandBack_0");
    auto curvedBench9BackCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench9BackCollider->position = scene.building->position + (curvedBench9Back.center * scene.building->scale);
    curvedBench9BackCollider->boxSize = curvedBench9Back.size;
    curvedBench9BackCollider->scale = scene.building->scale;
    curvedBench9BackCollider->shouldRender = false;
    curvedBench9BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench9BackCollider);

    // Curved_Bench_8_M_BenchEnd_0
    auto curvedBench8End = scene.building->getMesh()->getMeshBounds("Curved_Bench_8_M_BenchEnd_0");
    auto curvedBench8EndCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench8EndCollider->position = scene.building->position + (curvedBench8End.center * scene.building->scale);
    curvedBench8EndCollider->boxSize = curvedBench8End.size;
    curvedBench8EndCollider->scale = scene.building->scale;
    curvedBench8EndCollider->shouldRender = false;
    curvedBench8EndCollider->hasCollision = true;
    scene.objects.push_back(curvedBench8EndCollider);
    // Curved_Bench_8_M_BenchSeatandPicture_0
    auto curvedBench8Seat = scene.building->getMesh()->getMeshBounds("Curved_Bench_8_M_BenchSeatandPicture_0");
    auto curvedBench8SeatCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench8SeatCollider->position = scene.building->position + (curvedBench8Seat.center * scene.building->scale);
    curvedBench8SeatCollider->boxSize = curvedBench8Seat.size;
    curvedBench8SeatCollider->scale = scene.building->scale;
    curvedBench8SeatCollider->shouldRender = false;
    curvedBench8SeatCollider->hasCollision = true;
    scene.objects.push_back(curvedBench8SeatCollider);
    // Curved_Bench_8_M_BenchLowerandBack_0
    auto curvedBench8Back = scene.building->getMesh()->getMeshBounds("Curved_Bench_8_M_BenchLowerandBack_0");
    auto curvedBench8BackCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    curvedBench8BackCollider->position = scene.building->position + (curvedBench8Back.center * scene.building->scale);
    curvedBench8BackCollider->boxSize = curvedBench8Back.size;
    curvedBench8BackCollider->scale = scene.building->scale;
    curvedBench8BackCollider->shouldRender = false;
    curvedBench8BackCollider->hasCollision = true;
    scene.objects.push_back(curvedBench8BackCollider);

    // Large_Speaker_M_LargeSpeakerandGlasses_0
    auto largeSpeaker = scene.building->getMesh()->getMeshBounds("Large_Speaker_M_LargeSpeakerandGlasses_0");
    auto largeSpeakerCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    largeSpeakerCollider->position = scene.building->position + (largeSpeaker.center * scene.building->scale);
    largeSpeakerCollider->boxSize = largeSpeaker.size;
    largeSpeakerCollider->scale = scene.building->scale;
    largeSpeakerCollider->shouldRender = false;
    largeSpeakerCollider->hasCollision = true;
    scene.objects.push_back(largeSpeakerCollider);
    // Large_Speaker1_M_LargeSpeakerandGlasses_0
    auto largeSpeaker1 = scene.building->getMesh()->getMeshBounds("Large_Speaker1_M_LargeSpeakerandGlasses_0");
    auto largeSpeaker1Collider = std::make_shared<Object>(scene.building->getMesh(), "");
    largeSpeaker1Collider->position = scene.building->position + (largeSpeaker1.center * scene.building->scale);
    largeSpeaker1Collider->boxSize = largeSpeaker1.size;
    largeSpeaker1Collider->scale = scene.building->scale;
    largeSpeaker1Collider->shouldRender = false;
    largeSpeaker1Collider->hasCollision = true;
    scene.objects.push_back(largeSpeaker1Collider);

    // DJ_Stand_M_DJStandLower_0
    auto djStand = scene.building->getMesh()->getMeshBounds("DJ_Stand_M_DJStandLower_0");
    auto djStandCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    djStandCollider->position = scene.building->position + (djStand.center * scene.building->scale);
    djStandCollider->boxSize = djStand.size;
    djStandCollider->scale = scene.building->scale;
    djStandCollider->shouldRender = false;
    djStandCollider->hasCollision = true;
    scene.objects.push_back(djStandCollider);
    // DJ_Stand_M_DJstandUpper_0
    auto djStandUpper = scene.building->getMesh()->getMeshBounds("DJ_Stand_M_DJstandUpper_0");
    auto djStandUpperCollider = std::make_shared<Object>(scene.building->getMesh(), "");
    djStandUpperCollider->position = scene.building->position + (djStandUpper.center * scene.building->scale);
    djStandUpperCollider->boxSize = djStandUpper.size;
    djStandUpperCollider->scale = scene.building->scale;
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
    setupTimeline();

    // Start with timeline disabled by default
    scene.useTimeline = false;
    scene.allowManualCamera = true;

    scene.setupLighting();

    // Load cubemap
    std::vector<std::string> faces{
      "models/skybox/posx.png",
      "models/skybox/negx.png",
      "models/skybox/posy.png",
      "models/skybox/negy.png",
      "models/skybox/posz.png",
      "models/skybox/negz.png"
   };
    scene.skybox = std::make_unique<Skybox>(faces);
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

    // If not fullscreen, save as windowed size
    if (glfwGetWindowMonitor(window) == nullptr) {
      windowedWidth = width;
      windowedHeight = height;
    }

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

    // Resize post processing framebuffer
    if (postProcessing) {
      // Store current state
      auto currentMode = postProcessing->getMode();
      float bloomStrength = postProcessing->bloomStrength;
      float bloomThreshold = postProcessing->bloomThreshold;

      // Recreate with new size
      postProcessing = std::make_unique<PostProcessing>(width, height);

      // Restore state
      postProcessing->setMode(currentMode);
      postProcessing->setBloomStrength(bloomStrength);
      postProcessing->setBloomThreshold(bloomThreshold);

      // Reapply shadow map
      if (scene.shadowMap && currentMode == PostProcessingMode::SHADOW_DEBUG) {
        postProcessing->setShadowMapTexture(scene.shadowMap->getShadowTexture());
      }
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
    if (!freeCamera || scene.useTimeline) return;
    if (scene.useTimeline && scene.timeline && !scene.timeline->isPaused()) return;

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

  //Timeline story
  void setupTimeline() {
    scene.timeline = std::make_unique<Timeline>();

    //Camera timeline
    scene.timeline->addEvent(new CameraMovementEvent(
      0.0f,
      glm::vec3(0.0f, 1.2f, 6.0f),  // Position
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(-90.0f, -5.0f, 0.0f), // Rotation (yaw, pitch, roll)
      0.0f,
      "Init scene;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      0.1f,
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(-90.0f, -5.0f, 0.0f),
      3.0f,
      "Doors opening;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      3.5f,
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(-110.0f, -5.0f, 0.0f),
      1.5f,
      "Looking left;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      5.8f,
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(0.0f, 1.2f, 5.5f),
      glm::vec3(-70.0f, -5.0f, 0.0f),
      2.0f,
      "Looking right;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      8.0f,
      glm::vec3(2.3f, 1.2f, 2.5f),
      glm::vec3(1.0f, 1.2f, 5.5f),
      glm::vec3(2.0f, 1.2f, 5.5f),
      glm::vec3(-90.0f, -5.0f, 0.0f),
      6.0f,
      "Move right;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      13.8f,
      glm::vec3(2.3f, 1.2f, -0.6f),
      glm::vec3(2.3f, 1.2f, 1.5f),
      glm::vec3(2.3f, 1.2f, 0.5f),
      glm::vec3(-10.0f, -15.0f, 0.0f),
      4.0f,
      "Move forward, turn right;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      23.0f,
      glm::vec3(3.5f, 1.2f, -0.6f),
      glm::vec3(2.3f, 1.2f, -0.6f),
      glm::vec3(3.5f, 1.2f, -0.6f),
      glm::vec3(-180.0f, -5.0f, 0.0f),
      4.0f,
      "Turn around;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      27.0f,
      glm::vec3(-2.4f, 1.2f, -0.0f),
      glm::vec3(3.5f, 1.2f, -0.5f),
      glm::vec3(-2.4f, 1.2f, -0.1f),
      glm::vec3(-180.0f, -5.0f, 0.0f),
      10.0f,
      "Move forward;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      38.0f,
      glm::vec3(-2.8f, 1.f, -0.f),
      glm::vec3(-2.8f, 1.f, -0.f),
      glm::vec3(-2.8f, 1.f, -0.f),
      glm::vec3(-180.0f, -5.0f, 0.0f),
      0.0f,
      "Focus on glass;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      41.0f,
      glm::vec3(-2.98f, 0.98f, -0.18f),
      glm::vec3(-2.98f, 0.98f, -0.18f),
      glm::vec3(-2.98f, 0.98f, -0.18f),
      glm::vec3(-240.0f, -0.0f, 0.0f),
      0.0f,
      "Side perspective on glass;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      43.0f,
      glm::vec3(-3.2f, 1.3f, -0.8f),
      glm::vec3(-3.2f, 1.3f, -0.8f),
      glm::vec3(-3.2f, 1.3f, -0.8f),
      glm::vec3(60.0f, -20.0f, 0.0f),
      0.0f,
      "Perspective on main character;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
        45.0f,
      glm::vec3(0.0f, 1.2f, 0.3f),
      glm::vec3(0.0f, 1.2f, 0.3f),
      glm::vec3(0.0f, 1.2f, 0.3f),
      glm::vec3(-90.0f, -5.0f, 0.0f),
      0.0f,
      "Perspective on dancefloor;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
    51.0f,
      glm::vec3(-1.6f, 1.2f, -0.15f),
      glm::vec3(-1.6f, 1.2f, -0.15f),
      glm::vec3(-1.6f, 1.2f, -0.15f),
      glm::vec3(-180.0f, -70.0f, 0.0f),
      0.0f,
      "Perspective on dancefloor;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
    53.0f,
      glm::vec3(-2.1f, 0.5f, -0.15f),
      glm::vec3(-2.1f, 0.5f, -0.15f),
      glm::vec3(-2.1f, 0.5f, -0.15f),
      glm::vec3(0.0f, 50.0f, 0.0f),
      0.0f,
      "Perspective on dancefloor;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      55.0f,
      glm::vec3(0.0f, 1.8f, 3.0f),  // Position
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(-90.0f, -20.0f, 0.0f),
      0.0f,
      "Camera 1;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      75.0f,
      glm::vec3(-2.9f, 1.4f, 2.9f),  // Position
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(-50.0f, 0.0f, 0.0f),
      0.0f,
      "Camera 2;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      95.0f,
      glm::vec3(3.4f, 0.55, 2.6f),  // Position
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(-150.0f, 10.0f, 1.0f),
      0.0f,
      "Camera 3;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      115.0f,
      glm::vec3(0.1f, 2.3f, 0.9f),  // Position
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(90.0f, -90.0f, 0.0f),
      0.0f,
      "Camera 4;"
    ));

    scene.timeline->addEvent(new CameraMovementEvent(
      135.0f,
      glm::vec3(0.0f, 2.3f, -0.2f),  // Position
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(0.0f, 1.2f, 6.0f),
      glm::vec3(-90.0f, 0.0f, 0.0f),
      0.0f,
      "Camera 5;"
    ));

    //Man1 timeline
    scene.timeline->addEvent(new ObjectAnimationEvent(
      0.0f,
      "man1",
      0,
      "Man1; sit_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      0.0f,
      "man1",
      glm::vec3(3.7f, 0.1f, -1.8f),
      glm::vec3(0.0f, 5.5f, 0.0f),
      0.0f,
      "Init man1; "
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
      21.0f,
      "man1",
      1,
      "Man1; standup_animation"
    ));

    //Man2 timeline
    scene.timeline->addEvent(new ObjectAnimationEvent(
      0.0f,
      "man2",
      0,
      "Man2; sit_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      0.0f,
      "man2",
      glm::vec3(3.6f, 0.1f, 0.1f),
      glm::vec3(0.0f, 4.0f, 0.0f),
      0.0f,
      "Init man2; "
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
      21.5f,
      "man2",
      1,
      "Man2; standup_animation"
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
      41.0f,
      "man2",
      0,
      "Man2; sit_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      41.0f,
      "man2",
      glm::vec3(-2.4f, 0.35f, 1.2f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      0.0f,
      "Man2 bar"
    ));

    //Woman timeline
    scene.timeline->addEvent(new ObjectAnimationEvent(
      0.0f,
      "woman",
      0,
      "Woman; sit_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      0.0f,
      "woman",
      glm::vec3(4.0f, 0.1f, -0.8f),
      glm::vec3(0.0f, 4.8f, 0.0f),
      0.0f,
      "Init woman; "
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
      22.0f,
      "woman",
      1,
      "Woman; standup_animation"
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
      41.0f,
      "woman",
      0,
      "Woman; standup_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      41.0f,
      "woman",
      glm::vec3(-2.4f, 0.35f, 2.0f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      0.0f,
      "Woman bar"
    ));

    //Barman timeline
    scene.timeline->addEvent(new ObjectAnimationEvent(
      0.0f,
      "barman",
      0,
      "Barman; bartending_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      0.0f,
      "barman",
      glm::vec3(-3.9f, 0.1f, 0.0f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      0.0f,
      "Init barman;"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      36.0f,
      "barman",
      glm::vec3(-3.3f, 0.1f, 0.1f),
      glm::vec3(0.0f, 7.5f, 0.0f),
      1.5f,
      "Barman turn around;"
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
      38.0f,
      "barman",
      1,
      "Barman; putdown_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      38.0f,
      "barman",
      glm::vec3(-3.75f, 0.03f, -0.005f),
      glm::vec3(-1.6f, 0.0f, 1.5f),
      0.0f,
      "Remove barman from scene"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      41.0f,
      "barman",
      glm::vec3(-3.75f, -5.03f, 0.15f),
      glm::vec3(-1.6f, 0.0f, 1.5f),
      0.0f,
      "Remove barman from scene"
    ));

    //Main man timeline
    scene.timeline->addEvent(new ObjectAnimationEvent(
      0.0f,
      "man",
      0,
      "Main; sit_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      0.0f,
      "man",
      glm::vec3(-2.4f, -5.35f, 0.2f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      0.0f,
      "Init main;"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      43.0f,
      "man",
      glm::vec3(-2.4f, 0.35f, 0.2f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      0.0f,
      "Main bar;"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      44.0f,
      "man",
      glm::vec3(-2.4f, 0.35f, 0.2f),
      glm::vec3(0.0f, 2.5f, 0.0f),
      1.0f,
      "Main on the bar counter;"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      46.0f,
      "man",
      glm::vec3(-2.0f, 0.0f, -0.3f),
      glm::vec3(0.0f, -5.0f, 0.0f),
      0.0f,
      "Main on the bar counter;"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      47.0f,
      "man",
      glm::vec3(0.05f, 0.0f, -0.3f),
      glm::vec3(0.0f, -5.0f, 0.0f),
      1.0f,
      "Main on the bar counter;"
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
      47.5f,
      "man",
      2,
      "Main; sit_animation"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      48.0f,
      "man",
      glm::vec3(-0.3f, 0.0f, -0.3f),
      glm::vec3(0.0f, -4.7f, 0.0f),
      1.0f,
      "Main on the bar counter;"
    ));

    //Mad man timeline
    scene.timeline->addEvent(new ObjectMovementEvent(
      0.0f,
      "madMan",
      glm::vec3(0.4f, -5.0f, 0.0f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      0.0f,
      "Init madMan;"
    ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      46.0f,
      "madMan",
      glm::vec3(1.4f, 0.0f, -0.3f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      0.0f,
      "Init main;"
    ));

    scene.timeline->addEvent(new ObjectAnimationEvent(
        47.f,
        "madMan",
        0,
        "MadMan; punch_animation"
      ));

    scene.timeline->addEvent(new ObjectMovementEvent(
      47.0f,
      "madMan",
      glm::vec3(0.4f, 0.0f, -0.3f),
      glm::vec3(0.0f, 5.0f, 0.0f),
      1.0f,
      "Init main;"
    ));
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

    // Initialize post-processing BEFORE scene (needs OpenGL context)
    postProcessing = std::make_unique<PostProcessing>(WIDTH, HEIGHT);

    // Initialize shadow mapping
    scene.shadowMap = std::make_unique<ShadowMap>(2048, 2048);
    std::cout << "Shadow mapping initialized" << std::endl;

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

      // Post-processing mode switching
      if (key == GLFW_KEY_1) {
        postProcessing->setMode(PostProcessingMode::NONE);
        std::cout << "Post-processing: NONE" << std::endl;
      }
      if (key == GLFW_KEY_2) {
        postProcessing->setMode(PostProcessingMode::GRAYSCALE);
        std::cout << "Post-processing: GRAYSCALE" << std::endl;
      }
      if (key == GLFW_KEY_3) {
        postProcessing->setMode(PostProcessingMode::BLUR);
        std::cout << "Post-processing: BLUR" << std::endl;
      }
      if (key == GLFW_KEY_4) {
        postProcessing->setMode(PostProcessingMode::BLOOM);
        std::cout << "Post-processing: BLOOM" << std::endl;
      }
      if (key == GLFW_KEY_5) {
        postProcessing->setMode(PostProcessingMode::BLOOM_DEBUG);
        std::cout << "Post-processing: BLOOM DEBUG (shows only bright areas)" << std::endl;
      }
      if (key == GLFW_KEY_6) {
        if (scene.shadowMap) {
          postProcessing->setShadowMapTexture(scene.shadowMap->getShadowTexture());
          postProcessing->setMode(PostProcessingMode::SHADOW_DEBUG);
          std::cout << "Post-processing: SHADOW MAP DEBUG (visualizing depth buffer)" << std::endl;
        } else {
          std::cout << "Shadow map not available!" << std::endl;
        }
      }

      // Bloom adjustment controls
      if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {  // + key
        postProcessing->setBloomStrength(0.5f);
        std::cout << "Bloom strength increased to 0.5" << std::endl;
      }
      if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {  // - key
        postProcessing->setBloomStrength(0.1f);
        std::cout << "Bloom strength decreased to 0.1" << std::endl;
      }
      if (key == GLFW_KEY_LEFT_BRACKET) {  // [ key - lower threshold (more bloom)
        postProcessing->setBloomThreshold(0.5f);
        std::cout << "Bloom threshold lowered to 0.5" << std::endl;
      }
      if (key == GLFW_KEY_RIGHT_BRACKET) {  // ] key - raise threshold (less bloom)
        postProcessing->setBloomThreshold(1.0f);
        std::cout << "Bloom threshold raised to 1.0" << std::endl;
      }

      if (key == GLFW_KEY_F) {
        // Toggle fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (glfwGetWindowMonitor(window) == nullptr) {
          // Save current windowed size and position
          glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
          glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

          // Go fullscreen
          glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
          std::cout << "Fullscreen mode: " << mode->width << "x" << mode->height << std::endl;
        } else {
          // Go windowed - restore previous size
          glfwSetWindowMonitor(window, nullptr, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
          std::cout << "Windowed mode: " << windowedWidth << "x" << windowedHeight << std::endl;
        }
      }

      if (key == GLFW_KEY_T) {
        scene.useTimeline = !scene.useTimeline;
        scene.allowManualCamera = !scene.useTimeline;
        std::cout << "Timeline: " << (scene.useTimeline ? "ON" : "OFF") << std::endl;

        if (scene.useTimeline && scene.timeline) {
          scene.timeline->initializeCameraPosition(scene);
          scene.timeline->initializeObjectPositions(scene);
          scene.timeline->initializeAnimations(scene);
          scene.timeline->printUpcomingEvents(20.0f);
        }
      }

      if (key == GLFW_KEY_SPACE) {
        if (scene.timeline) {
          scene.timeline->reset();

          scene.timeline->initializeCameraPosition(scene);
          scene.timeline->initializeObjectPositions(scene);
          scene.timeline->initializeAnimations(scene);

          scene.useTimeline = true;
          scene.allowManualCamera = false;
          std::cout << "Timeline restarted!" << std::endl;
        }
      }

      if (key == GLFW_KEY_ENTER) {
        if (scene.timeline) {
          if (scene.timeline->isPaused()) {
            scene.timeline->resume();
            scene.allowManualCamera = false;
          } else {
            scene.timeline->pause();
            scene.allowManualCamera = true;
          }
        }
      }
    }
  }

  void onIdle() override {
    // Track time
    float currentTime = (float)glfwGetTime();
    float dt = currentTime - time;
    time = currentTime;

    // ========== SHADOW PASS ==========
    if (scene.shadowMap && scene.shadowMap->isEnabled()) {
      // Begin shadow pass - render scene from light's perspective
      glm::vec3 sceneCenter(0.0f, 2.0f, 0.0f);  // Center of your scene
      float sceneRadius = 25.0f;  // Adjust based on your scene size

      scene.shadowMap->beginShadowPass(
        scene.directionalLight.direction,
        sceneCenter,
        sceneRadius
      );

      // Render all objects to shadow map
      auto shadowShader = scene.shadowMap->getShadowShader();
      shadowShader->use();

      for (auto& obj : scene.objects) {
        // Skip if no mesh, not rendering, transparent, or billboard
        if (!obj->mesh || !obj->shouldRender || obj->forceTransparent || obj->billboarding) {
          continue;
        }

        shadowShader->setUniform("ModelMatrix", obj->modelMatrix);
        shadowShader->setUniform("lightSpaceMatrix", scene.shadowMap->getLightSpaceMatrix());

        // Render all meshes of this object
        size_t meshCount = obj->mesh->getMeshCount();
        for (size_t i = 0; i < meshCount; ++i) {
          // Skip transparent meshes
          if (obj->isMeshTransparent(obj->mesh->getMeshName(i))) {
            continue;
          }
          obj->mesh->renderSingleMesh(i);
        }
      }

      scene.shadowMap->endShadowPass();
    }

    // ========== NORMAL RENDER PASS ==========
    // Restore viewport for main rendering
    glViewport(0, 0, currentWidth, currentHeight);

    // Begin rendering to post-processing framebuffer
    if (postProcessing) {
      postProcessing->beginFrame();
    }

    glClearColor(.2f, .2f, .2f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.skybox->render(scene.camera->viewMatrix, scene.camera->projectionMatrix);

    scene.update(dt);
    scene.render();  // This now includes shadows

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

    // End framebuffer rendering and apply post-processing
    if (postProcessing) {
      postProcessing->endFrame();
      postProcessing->render();
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
  std::cout << "T - Toggle timeline" << std::endl;
  std::cout << "SPACE - Start/Restart timeline" << std::endl;
  std::cout << "ENTER - Pause/Resume timeline" << std::endl;
  std::cout << "1 - No post-processing" << std::endl;
  std::cout << "2 - Grayscale filter" << std::endl;
  std::cout << "3 - Blur effect" << std::endl;
  std::cout << "4 - Bloom effect" << std::endl;
  std::cout << "5 - Bloom debug (shows bright areas only)" << std::endl;
  std::cout << "6 - Shadow map debug (visualize depth buffer)" << std::endl;
  std::cout << "+/- - Increase/Decrease bloom strength" << std::endl;
  std::cout << "[/] - Lower/Raise bloom threshold" << std::endl;
  std::cout << "=================" << std::endl;

  SceneWindow window;

  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}