#include "component_trans.h"

#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "pybind/pybind.h"

namespace ECS {
glm::mat4 ComponentTransform::GetTransform() const {
  auto trans_mat = glm::translate(glm::mat4(1.0f), _translation);
  auto rotate_mat = glm::toMat4(_rotation);
  auto scale_mat = glm::scale(glm::mat4(1.0f), _scale);

  return trans_mat * rotate_mat * scale_mat;
}

void ComponentTransform::UpdateLastTrans() {
  _first_get_last_trans = false;
  _last_trans = GetTransform();
}

glm::mat4 ComponentTransform::GetLastTrans() {
  if (_first_get_last_trans) {
    return GetTransform();
  }
  return _last_trans;
}

glm::vec3 ComponentTransform::GetPosition() const { return _translation; }

void ComponentTransform::SetPosition(glm::vec3 pos) { _translation = pos; }

glm::quat ComponentTransform::GetRotation() const { return _rotation; }

glm::vec3 ComponentTransform::GetRotationEular() const
{
  return glm::degrees(glm::eulerAngles(_rotation));
}

glm::vec3 ComponentTransform::GetScale() const { return _scale; }

void ComponentTransform::SetRotation(const glm::quat& q)
{
  _rotation = q;
}

void ComponentTransform::SetRotationEular(const glm::vec3& eular)
{
  _rotation = glm::quat(glm::vec3(eular.y, eular.x, eular.z));
}

void ComponentTransform::SetForward(const glm::vec3& forward)
{
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  if (glm::length(glm::cross(up, forward)) < 0.0001f) {
    up = glm::vec3(1.0f, 0.0f, 0.0f);
  }
  auto view = glm::lookAt(GetPosition(), GetPosition() - forward, up);
  SetTransform(glm::inverse(view));
}

bool ComponentTransform::SetTransform(glm::mat4 trans_mat) {
  glm::vec3 skew;
  glm::vec4 perspective;
  return glm::decompose(trans_mat, _scale, _rotation, _translation, skew,
                        perspective);
  using type_new = decltype(&ComponentTransform::GetPosition);
  using ttt_type = std::remove_const_t<type_new>;
}

BIND_CLS_FUNC_DEFINE(ComponentTransform, SetPosition)
BIND_CLS_FUNC_DEFINE(ComponentTransform, GetPosition)
BIND_CLS_FUNC_DEFINE(ComponentTransform, SetScale)
BIND_CLS_FUNC_DEFINE(ComponentTransform, GetScale)
BIND_CLS_FUNC_DEFINE(ComponentTransform, GetRotation)
BIND_CLS_FUNC_DEFINE(ComponentTransform, SetRotation)
BIND_CLS_FUNC_DEFINE(ComponentTransform, SetRotationEular)
BIND_CLS_FUNC_DEFINE(ComponentTransform, SetForward)
BIND_CLS_FUNC_DEFINE(ComponentTransform, UpdateLastTrans)

static PyMethodDef type_methods[] = {
  {"SetPosition", BIND_CLS_FUNC_NAME(ComponentTransform, SetPosition), METH_VARARGS, 0},
  {"GetPosition", BIND_CLS_FUNC_NAME(ComponentTransform, GetPosition), METH_NOARGS, 0},
  {"SetScale", BIND_CLS_FUNC_NAME(ComponentTransform, SetScale), METH_VARARGS, 0},
  {"GetScale", BIND_CLS_FUNC_NAME(ComponentTransform, GetScale), METH_NOARGS, 0},
  {"GetRotation", BIND_CLS_FUNC_NAME(ComponentTransform, GetRotation), METH_NOARGS, 0},
  {"SetRotation", BIND_CLS_FUNC_NAME(ComponentTransform, SetRotation), METH_VARARGS, 0},
  {"SetRotationEular", BIND_CLS_FUNC_NAME(ComponentTransform, SetRotationEular), METH_VARARGS, 0},
  {"SetForward", BIND_CLS_FUNC_NAME(ComponentTransform, SetForward), METH_VARARGS, 0},
  {"UpdateLastTrans", BIND_CLS_FUNC_NAME(ComponentTransform, UpdateLastTrans), METH_NOARGS, 0},
  {0, nullptr, 0, 0},
};

DEFINE_PYCXX_OBJECT_TYPE(Component, ComponentTransform, "ComponentTransform", type_methods, py_init_params<>())
} // namespace ECS
