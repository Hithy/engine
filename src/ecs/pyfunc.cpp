#include "pyfunc.h"

#include "entity_base.h"
#include "scene.h"
#include "component_camera.h"
#include "component_light.h"
#include "component_model.h"
#include "component_trans.h"

#include "pybind/pybind.h"

namespace ECS {
  Entity* CreateEntity()
  {
    auto ent = new Entity();
    ent->SetRef(0);
    return ent;
  }

  ComponentModel* CreateComponentModel(const char* path)
  {
    auto res = new ComponentModel(path);
    res->SetRef(0);
    return res;
  }

  ComponentTransform* CreateComponentTransform()
  {
    auto res = new ComponentTransform();
    res->SetRef(0);
    return res;
  }

  ComponentCamera* CreateComponentCamera()
  {
    auto res = new ComponentCamera();
    res->SetRef(0);
    return res;
  }

  ComponentLight* CreateComponentLight(int light_type)
  {
    auto res = new ComponentLight(static_cast<LightType>(light_type));
    res->SetRef(0);
    return res;
  }

  Scene* CreateScene()
  {
    auto res = new Scene();
    res->SetRef(0);
    return res;
  }

  BIND_FUNC_DEFINE(CreateEntity);
  BIND_FUNC_DEFINE(CreateComponentModel);
  BIND_FUNC_DEFINE(CreateComponentTransform);
  BIND_FUNC_DEFINE(CreateComponentCamera);
  BIND_FUNC_DEFINE(CreateComponentLight);

  static PyMethodDef my_methods[] = {
  {"CreateEntity", BIND_FUNC_NAME(CreateEntity), METH_NOARGS, NULL},
  {"CreateComponentModel", BIND_FUNC_NAME(CreateComponentModel), METH_VARARGS, NULL},
  {"CreateComponentTransform", BIND_FUNC_NAME(CreateComponentTransform), METH_NOARGS, NULL},
  {"CreateComponentCamera", BIND_FUNC_NAME(CreateComponentCamera), METH_NOARGS, NULL},
  {"CreateComponentLight", BIND_FUNC_NAME(CreateComponentLight), METH_VARARGS, NULL},
  {nullptr, 0, 0, 0}
  };

  void InitFuncModule(void* mod)
  {
    PyModule_AddFunctions((PyObject *)mod, my_methods);
  }

};