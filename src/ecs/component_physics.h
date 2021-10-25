#pragma once
#include "component_base.h"

#include <PxPhysicsAPI.h>

#include <glm/glm.hpp>

namespace ECS {
class ComponentPhysics : public Component {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(ComponentPhysics)
  ComponentPhysics(bool is_static, int geo_type, const glm::vec3& geo_size)
    : Component(ComponentType_Physics)
    , _is_static(is_static)
    , _body(nullptr)
    , _geo_type(geo_type)
    , _geo_size(geo_size)
    , _is_kinematic(false)
  {
  };

  bool IsKinematic() { return _is_kinematic; }
  void SetKinematic(bool enable);
  bool IsStatic() { return _is_static; }
  bool IsLoaded() { return _body != nullptr; }
  void SetActor(physx::PxRigidActor* new_body);

  int GetGeoType() { return _geo_type; }
  glm::vec3 GetGeoSize() { return _geo_size; }

  glm::mat4 GetTransform();
  glm::vec3 GetPosition();
  glm::quat GetRotation();

  void AddForce(const glm::vec3& force);

private:
  physx::PxRigidActor* _body;
  bool _is_static;
  bool _is_kinematic;
  int _geo_type;
  glm::vec3 _geo_size;
};
} // namespace ECS
