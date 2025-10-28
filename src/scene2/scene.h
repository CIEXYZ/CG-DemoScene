#ifndef _PPGSO_SCENE_H
#define _PPGSO_SCENE_H

#include <memory>
#include <map>
#include <list>

#include "object.h"
#include "camera.h"
#include "grid.h"

class Scene {
public:
  void update(float dt);
  void updateCamera(float dt);

  void render();
  void renderGrid();

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