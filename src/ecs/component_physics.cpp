#include "component_physics.h"

#include "pybind/pybind.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace ECS {
  void ComponentPhysics::SetKinematic(bool enable)
  {
    _is_kinematic = enable;
    if (!_is_static && _body) {
      auto rigid_body = static_cast<physx::PxRigidBody*>(_body);
      if (rigid_body) {
        rigid_body->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, enable);
      }
    }
  }
  void ComponentPhysics::SetActor(physx::PxRigidActor* new_body) {
  if (_body) {
    _body->release();
  }
  _body = new_body;
}

glm::mat4 ComponentPhysics::GetTransform()
{
  auto px_trans = _body->getGlobalPose();
  auto trans = glm::translate(glm::mat4(1.0f), glm::vec3(px_trans.p.x, px_trans.p.y, px_trans.p.z));
  auto rotate = glm::toMat4(glm::quat(px_trans.q.x, px_trans.q.y, px_trans.q.z, px_trans.q.w));
  return trans * rotate;
}

glm::vec3 ComponentPhysics::GetPosition()
{
  auto px_trans = _body->getGlobalPose();
  return glm::vec3(px_trans.p.x, px_trans.p.y, px_trans.p.z);
}

glm::quat ComponentPhysics::GetRotation()
{
  auto px_trans = _body->getGlobalPose();
  return glm::quat(px_trans.q.x, px_trans.q.y, px_trans.q.z, px_trans.q.w);
}

void ComponentPhysics::AddForce(const glm::vec3& force)
{
  if (!_is_static && _body) {
    auto rigid_body = static_cast<physx::PxRigidBody*>(_body);
    rigid_body->addForce(physx::PxVec3(force.x, force.y, force.z));
  }
}

BIND_CLS_FUNC_DEFINE(ComponentPhysics, SetKinematic)
BIND_CLS_FUNC_DEFINE(ComponentPhysics, IsKinematic)
BIND_CLS_FUNC_DEFINE(ComponentPhysics, AddForce)

static PyMethodDef type_methods[] = {
  {"SetKinematic", BIND_CLS_FUNC_NAME(ComponentPhysics, SetKinematic), METH_VARARGS, 0},
  {"IsKinematic", BIND_CLS_FUNC_NAME(ComponentPhysics, IsKinematic), METH_VARARGS, 0},
  {"AddForce", BIND_CLS_FUNC_NAME(ComponentPhysics, AddForce), METH_VARARGS, 0},
  {0, nullptr, 0, 0},
};

DEFINE_PYCXX_OBJECT_TYPE(Component, ComponentPhysics, "ComponentPhysics", type_methods, py_init_params<bool COMMA int COMMA const glm::vec3&>())


} // namespace ECS
