#include "pymath.h"

#include <glm/gtx/euler_angles.hpp>

#include "pybind/pybind.h"

namespace ECS {

  BIND_FUNC_DEFINE(GenRotation);
  BIND_FUNC_DEFINE(Rotate);

  static PyMethodDef my_methods[] = {
  {"GenRotation", BIND_FUNC_NAME(GenRotation), METH_VARARGS, NULL},
  {"Rotate", BIND_FUNC_NAME(Rotate), METH_VARARGS, NULL},
  {nullptr, 0, 0, 0}
  };

  glm::quat GenRotation(const glm::vec3& axis, float degree)
  {
    return glm::rotate(glm::mat4(1.0f), glm::radians(degree), axis);
  }

  glm::quat Rotate(const glm::quat& from, const glm::quat& delta)
  {
    return delta * from;
  }

  void InitMathModule(void* mod)
  {
    PyModule_AddFunctions((PyObject *)mod, my_methods);
  }

};