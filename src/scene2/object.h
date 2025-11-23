#pragma once

#include <ppgso/ppgso.h>
#include <glm/glm.hpp>
#include <memory>
#include <map>
#include "Mesh_Assimp.h"

class Scene;

class Object {
public:
  glm::vec3 position{0,0,0};
  glm::vec3 rotation{0,0,0};
  glm::vec3 scale{1,1,1};
  glm::mat4 modelMatrix{1};
  bool billboarding = false;
  float transparency = 1.f;

  Object() = default;
  Object(const std::string& modelPath);
  Object(std::shared_ptr<ppgso::Mesh_Assimp> sharedMesh, const std::string& texturePath);
  virtual ~Object() = default;

  virtual void update(Scene &scene, float dt);
  virtual void render(Scene &scene);

  void setMesh(const std::string& meshPath);
  void setShader(const std::string& vertShader, const std::string& fragShader);

  std::shared_ptr<ppgso::Mesh_Assimp> getMesh() const { return mesh; }

protected:
  void generateModelMatrix();

  std::shared_ptr<ppgso::Mesh_Assimp> mesh;
  std::unique_ptr<ppgso::Shader> shader;
  std::map<unsigned int, std::unique_ptr<ppgso::Texture>> materialTextures;

  bool ownsMesh = true;
};