#pragma once

#include <ppgso/ppgso.h>
#include <glm/glm.hpp>
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <set>
#include "Mesh_Assimp.h"
#include "AABB.h"

class Scene;

class Object {
public:
  glm::vec3 position{0,0,0};
  glm::vec3 rotation{0,0,0};
  glm::vec3 scale{1,1,1};
  glm::mat4 modelMatrix{1};
  glm::vec3 velocity{0.f};
  glm::vec3 oldPosition{0.f};

  bool shouldRender = true;
  float transparency = 1.f;
  float drag = 1.f;
  float mass = 1.f;
  bool billboarding = false;
  bool hasCollision = false; //turn on for objects that need it
  bool usesGravity = false;
  bool isGrounded = false;
  bool forceTransparent = false;
  bool isParticle = false;
  bool useNormalMapping = false;

  AABB boundingBox;

  glm::vec3 boxSize = glm::vec3(1.f);
  glm::vec3 boxOffset = glm::vec3(0.0f);
  glm::vec3 acceleration = glm::vec3(0.0f);
  glm::vec3 forces = glm::vec3(0.0f);

  glm::vec3 angularVelocity = glm::vec3(0.0f);
  glm::vec3 torque = glm::vec3(0.0f);
  float momentOfInertia = 1.0f;


  Object() = default;
  Object(const std::string& modelPath);
  Object(std::shared_ptr<ppgso::Mesh_Assimp> sharedMesh, const std::string& texturePath);
  virtual ~Object() = default;

  // Set which mesh names should be transparent
  void setTransparentMeshes(const std::vector<std::string>& meshNames);
  bool isMeshTransparent(const std::string& meshName) const;

  virtual void update(Scene &scene, float dt);
  virtual void stopOnYAxis(std::shared_ptr<Object> &obj);
  virtual void stopOnXAxis(std::shared_ptr<Object> &obj);
  virtual void stopOnZAxis(std::shared_ptr<Object> &obj);
  virtual void render(Scene &scene);

  void setNormalMap(unsigned int materialIndex, const std::string& normalMapPath);
  void setMesh(const std::string& meshPath);
  void setShader(const std::string& vertShader, const std::string& fragShader);

  std::shared_ptr<ppgso::Mesh_Assimp> getMesh() const { return mesh; }

  bool collidesWithOther(const Object &otherObject) const;
  void applyForces(const glm::vec3 &force);
  void clearForce();
  void applyTorque(const glm::vec3& t) { torque += t; }
  void clearTorque() { torque = glm::vec3(0.0f); }


// protected:
  void generateModelMatrix();

  std::shared_ptr<ppgso::Mesh_Assimp> mesh;
  std::unique_ptr<ppgso::Shader> shader;
  std::map<unsigned int, std::shared_ptr<ppgso::Texture>> materialTextures;
  std::map<unsigned int, std::shared_ptr<ppgso::Texture>> normalMaps;
  std::map<unsigned int, std::shared_ptr<ppgso::Texture>> specularMaps;

  bool ownsMesh = true;
  std::set<std::string> transparentMeshNames;
};