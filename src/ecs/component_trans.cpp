#include "component_trans.h"

#include "glm/gtx/quaternion.hpp"
#include <glm/gtx/matrix_decompose.hpp>

namespace ECS {
glm::mat4 ComponentTransform::GetTransform() const {
  auto trans_mat = glm::translate(glm::mat4(1.0f), _translation);
  auto rotate_mat = glm::toMat4(_rotation);
  auto scale_mat = glm::scale(glm::mat4(1.0f), _scale);

  return trans_mat * rotate_mat * scale_mat;
}

glm::vec3 ComponentTransform::GetPosition() const { return _translation; }

void ComponentTransform::SetPosition(glm::vec3 pos) { _translation = pos; }

glm::quat ComponentTransform::GetRotation() const { return _rotation; }

glm::vec3 ComponentTransform::GetScale() const { return _scale; }

bool ComponentTransform::SetTransform(glm::mat4 trans_mat) {
  glm::vec3 skew;
  glm::vec4 perspective;
  return glm::decompose(trans_mat, _scale, _rotation, _translation, skew,
                        perspective);
}
} // namespace ECS
