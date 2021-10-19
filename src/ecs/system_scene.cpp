#include "system_scene.h"

#include "pybind/pybind.h"
#include "scene.h"

namespace ECS {
  Scene* System::GetScene()
  {
    return _scene;
  }

  BIND_CLS_FUNC_DEFINE(System, ScriptTick);
  BIND_CLS_FUNC_DEFINE(System, ScriptStart);
  BIND_CLS_FUNC_DEFINE(System, ScriptStop);
  BIND_CLS_FUNC_DEFINE(System, GetScene);

  static PyMethodDef type_methods[] = {
    {"tick", BIND_CLS_FUNC_NAME(System, ScriptTick), METH_VARARGS, 0},
    {"start", BIND_CLS_FUNC_NAME(System, ScriptStart), METH_NOARGS, 0},
    {"stop", BIND_CLS_FUNC_NAME(System, ScriptStop), METH_NOARGS, 0},
    {"GetScene", BIND_CLS_FUNC_NAME(System, GetScene), METH_NOARGS, 0},
    {0, nullptr, 0, 0},
  };

  DEFINE_PYCXX_OBJECT_TYPE_BASE(System, "System", type_methods, py_init_params<int>());
} // namespace ECS
