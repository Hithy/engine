#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace ECS {

  glm::quat GenRotation(const glm::vec3& axis, float degree);
  glm::quat Rotate(const glm::quat& from, const glm::quat& delta);

  void InitMathModule(void* mod);
}