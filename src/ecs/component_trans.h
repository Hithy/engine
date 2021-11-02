#pragma once

#include "component_base.h"
#include "glm/ext/quaternion_float.hpp"
#include "glm/glm.hpp"

namespace ECS {
class ComponentTransform : public Component {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(ComponentTransform);
  ComponentTransform()
      : Component(ComponentType_Transform)
      , _scale(glm::vec3(1.0f, 1.0f, 1.0f))
      , _rotation(glm::quat(0.0f, 0.0f, 0.0f, 1.0f))
      , _translation(glm::vec3(0.0f, 0.0f, 0.0f))
      , _first_get_last_trans(true)
  {
    SetTransform(glm::mat4(1.0f));
  };

  ComponentTransform(glm::mat4 trans)
      : Component(ComponentType_Transform){
    SetTransform(trans);
  };

  void SetPosition(glm::vec3);
  glm::vec3 GetPosition() const;

  glm::quat GetRotation() const;
  glm::vec3 GetRotationEular() const;

  void SetScale(glm::vec3 scale) { _scale = scale; }
  glm::vec3 GetScale() const;

  void SetRotation(const glm::quat& q);
  void SetRotationEular(const glm::vec3& eular);

  void SetForward(const glm::vec3& forward);

  bool SetTransform(glm::mat4 trans_mat);
  glm::mat4 GetTransform() const;

  void UpdateLastTrans();
  glm::mat4 GetLastTrans();

private:
  glm::vec3 _scale;
  glm::quat _rotation;
  glm::vec3 _translation;

  glm::mat4 _last_trans;
  bool _first_get_last_trans;
};
} // namespace ECS
