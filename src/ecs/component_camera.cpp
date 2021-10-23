#include "component_camera.h"
#include "component_trans.h"
#include "entity_base.h"

#include <glm/gtc/matrix_transform.hpp>
#include "pybind/pybind.h"

namespace ECS {
glm::mat4 ComponentCamera::GetView() {
  auto yaw_rot = glm::rotate(glm::mat4(1.0f), glm::radians(-_yaw),
                             glm::vec3(0.0f, 1.0f, 0.0f));
  auto new_right_vec = yaw_rot * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
  auto pitch_rot = glm::rotate(glm::mat4(1.0f), glm::radians(_pitch),
                               glm::vec3(new_right_vec));
  auto rotate_mat = glm::inverse(pitch_rot * yaw_rot);

  auto ent = dynamic_cast<Entity*>(GetEntity());
  if (ent) {
    auto comp_trans = dynamic_cast<ComponentTransform *>(
        ent->GetComponent(ComponentType_Transform));

    if (comp_trans) {
      auto pos = comp_trans->GetPosition();
      return rotate_mat * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    }
  }

  return rotate_mat * glm::mat4(1.0f);
}

glm::mat4 ComponentCamera::GetProjection()
{
    return glm::perspective(glm::radians(_fov), _ratio, 0.1f, 1000.0f);
}

BIND_CLS_FUNC_DEFINE(ComponentCamera, Lock);

static PyMethodDef type_methods[] = {
  {"Lock", BIND_CLS_FUNC_NAME(ComponentCamera, Lock), METH_VARARGS, 0},
  {0, nullptr, 0, 0},
};

DEFINE_PYCXX_OBJECT_TYPE(Component, ComponentCamera, "ComponentCamera", type_methods, py_init_params<>())
} // namespace ECS
