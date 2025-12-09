#ifndef _PPGSO_SCENE_H
#define _PPGSO_SCENE_H

#include <memory>
#include <map>
#include <list>

#include "object.h"
#include "camera.h"
#include "grid.h"
#include "lighting.h"
#include "Skybox.h"

class Timeline;

class Scene {
public:
  void update(float dt);
  void updateCamera(float dt);

  void render();
  void renderGrid();

  Material defaultMaterial;

  DirectionalLight directionalLight;

  PointLight redLight;
  PointLight greenLight;
  PointLight blueLight;
  PointLight ceilingLight;

  SpotLight djSpot;
  SpotLight danceSpot;

  std::shared_ptr<Object> building; // needed for referencing building for light positions

  bool useDirLight = true;
  bool usePointLight = true;
  bool useSpotLight = true;

  bool useBlinnPhong = true;

  void setupLighting();
  void updateLights(float dt);

  std::unique_ptr<Camera> camera;

  std::unique_ptr<Grid> grid;

  std::unique_ptr<Timeline> timeline;

  std::list<std::shared_ptr<Object>> objects;

  std::unique_ptr<Skybox> skybox;

  std::map< int, int > keyboard;

  glm::vec3 lightDirection{-1.0f, -1.0f, -1.0f};

  struct {
    double x, y;
    bool left, right;
  } cursor;

  bool useTimeline = false;
  bool allowManualCamera = true;
};

#endif // _PPGSO_SCENE_H