#include "pyfunc.h"

#include "entity_base.h"
#include "scene.h"
#include "component_camera.h"
#include "component_light.h"
#include "component_model.h"
#include "component_trans.h"
#include "system_camera.h"
#include "system_model.h"
#include "system_input.h"
#include "system_syncrender.h"

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

  SystemCamera* CreateSystemCamera()
  {
    auto res = new SystemCamera();
    res->SetRef(0);
    return res;
  }

  SystemModel* CreateSystemModel()
  {
    auto res = new SystemModel();
    res->SetRef(0);
    return res;
  }

  SystemInput* CreateSystemInput()
  {
    auto res = new SystemInput();
    res->SetRef(0);
    return res;
  }

  SystemSyncRender* CreateSystemSyncRender()
  {
    auto res = new SystemSyncRender();
    res->SetRef(0);
    return res;
  }

  BIND_FUNC_DEFINE(CreateScene);
  BIND_FUNC_DEFINE(CreateEntity);
  BIND_FUNC_DEFINE(CreateComponentModel);
  BIND_FUNC_DEFINE(CreateComponentTransform);
  BIND_FUNC_DEFINE(CreateComponentCamera);
  BIND_FUNC_DEFINE(CreateComponentLight);
  BIND_FUNC_DEFINE(CreateSystemCamera);
  BIND_FUNC_DEFINE(CreateSystemModel);
  BIND_FUNC_DEFINE(CreateSystemInput);
  BIND_FUNC_DEFINE(CreateSystemSyncRender);

  static PyMethodDef my_methods[] = {
  {"CreateScene", BIND_FUNC_NAME(CreateScene), METH_NOARGS, NULL},
  {"CreateEntity", BIND_FUNC_NAME(CreateEntity), METH_NOARGS, NULL},
  {"CreateComponentModel", BIND_FUNC_NAME(CreateComponentModel), METH_VARARGS, NULL},
  {"CreateComponentTransform", BIND_FUNC_NAME(CreateComponentTransform), METH_NOARGS, NULL},
  {"CreateComponentLight", BIND_FUNC_NAME(CreateComponentLight), METH_VARARGS, NULL},
  {"CreateSystemCamera", BIND_FUNC_NAME(CreateSystemCamera), METH_NOARGS, NULL},
  {"CreateSystemModel", BIND_FUNC_NAME(CreateSystemModel), METH_NOARGS, NULL},
  {"CreateSystemInput", BIND_FUNC_NAME(CreateSystemInput), METH_NOARGS, NULL},
  {"CreateSystemSyncRender", BIND_FUNC_NAME(CreateSystemSyncRender), METH_NOARGS, NULL},
  {nullptr, 0, 0, 0}
  };

  void InitFuncModule(void* mod)
  {
    PyModule_AddFunctions((PyObject *)mod, my_methods);
  }

};