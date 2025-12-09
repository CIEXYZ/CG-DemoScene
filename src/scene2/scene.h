#ifndef _PPGSO_SCENE_H
#define _PPGSO_SCENE_H

#include <memory>
#include <map>
#include <list>

#include "object.h"
#include "camera.h"
#include "grid.h"
#include "lighting.h"

class Scene {
public:
  void update(float dt);
  void updateCamera(float dt);

  void render();
  void renderGrid();

  Material defaultMaterial;
  DirectionalLight directionalLight;
  std::vector<PointLight> pointLights;
  std::vector<SpotLight> spotLights;
  std::shared_ptr<Object> building; // needed for referencing building for light positions

  bool useDirLight = true;
  bool useBlinnPhong = true;

  void setupLighting();
  void updateLights(float dt);

  std::unique_ptr<Camera> camera;

  std::unique_ptr<Grid> grid;

  std::list<std::shared_ptr<Object>> objects;

  std::map< int, int > keyboard;

  glm::vec3 lightDirection{-1.0f, -1.0f, -1.0f};

  struct {
    double x, y;
    bool left, right;
  } cursor;
};

#endif // _PPGSO_SCENE_H