#include "python_ecs.h"
#include "pybind/pybind.h"

#include "ecs/entity_base.h"
#include "ecs/world.h"
#include "ecs/scene.h"
#include "ecs/component_camera.h"
#include "ecs/component_model.h"
#include "ecs/component_trans.h"
#include "ecs/component_base.h"
#include "ecs/component_light.h"

#include "ecs/system_camera.h"
#include "ecs/system_input.h"
#include "ecs/system_model.h"
#include "ecs/system_syncrender.h"
#include "ecs/system_scene.h"

#include "ecs/pyfunc.h"
#include "ecs/pymath.h"

#include <Python.h>
#include <vector>

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace ECS {

#ifdef WIN32
static void GetAbsPath(wchar_t* output, int buff_size, const char* sub_path) {
  char file_path[2048] = { 0 };
  GetCurrentDirectory(2048, file_path);

  strcat(file_path, "\\");
  strcat(file_path, sub_path);
  MultiByteToWideChar(CP_UTF8, 0, file_path, -1, output, buff_size);
}

static void InitPath() {
  wchar_t output[2048] = { 0 };
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "3rd\\python");
  Py_SetPythonHome(output);

  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "3rd\\python\\Lib");
  std::wstring path = output;
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "script");
  path = path + L";" + output;
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "3rd\\python\\Lib\\site-packages");
  path = path + L";" + output;
  Py_SetPath(path.c_str());
}
#else
static void GetAbsPath(wchar_t* output, int buff_size, const char* sub_path) {
  char file_path[2048] = { 0 };
  getcwd(file_path, buff_size);

  strcat(file_path, "/");
  strcat(file_path, sub_path);

  swprintf(output, buff_size, L"%s", file_path);
}

static void InitPath() {
  wchar_t output[2048] = { 0 };
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "3rd/python");
  Py_SetPythonHome(output);
  
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "3rd/python/Lib");
  std::wstring path = output;
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "script");
  path = path + L":" + output;
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "3rd/python/Lib/site-packages");
  path = path + L":" + output;
  Py_SetPath(path.c_str());
}
#endif

int cxx_test_add(short a, int b) {
  return a + b;
}

int cxx_test_sub_two(int a, int b, int c) {
  return a - b - c;
}

void print_list(const std::vector<int>& hello) {
  std::cout << "engine print list: ";
  for (int i = 0; i < hello.size(); i++) {
    std::cout << hello[i] << " ";
  }
  std::cout << "\n";
}

std::vector<uint64_t> get_scene_entities(Scene& scn) {
  return scn.GetEntityIds();
}

static World& GetWorldObj() {
  return World::GetInstance();
}

BIND_FUNC_DEFINE(cxx_test_add)
BIND_FUNC_DEFINE(cxx_test_sub_two)
BIND_FUNC_DEFINE(GetWorldObj)
BIND_FUNC_DEFINE(print_list)
BIND_FUNC_DEFINE(get_scene_entities)

static PyMethodDef my_methods[] = {
  {"test_add", BIND_FUNC_NAME(cxx_test_add), METH_VARARGS, NULL},
  {"test_sub2", BIND_FUNC_NAME(cxx_test_sub_two), METH_VARARGS, NULL},
  {"get_world", BIND_FUNC_NAME(GetWorldObj), METH_NOARGS, NULL},
  {"print_list", BIND_FUNC_NAME(print_list), METH_VARARGS, NULL},
  {"get_scene_entities", BIND_FUNC_NAME(get_scene_entities), METH_VARARGS, NULL},
  {nullptr, 0, 0, 0}
};

void InitPython() {
  InitPath();
  Py_Initialize();

  auto new_module = PyImport_AddModule("_engine");
  PyModule_AddFunctions(new_module, my_methods);
  PyModule_AddType(new_module, Entity::GetPyType());
  PyModule_AddType(new_module, Scene::GetPyType());
  PyModule_AddType(new_module, World::GetPyType());
  PyModule_AddType(new_module, System::GetPyType());
  PyModule_AddType(new_module, Component::GetPyType());

  PyModule_AddType(new_module, ComponentModel::GetPyType());
  PyModule_AddType(new_module, ComponentTransform::GetPyType());
  PyModule_AddType(new_module, ComponentCamera::GetPyType());
  PyModule_AddType(new_module, ComponentLight::GetPyType());

  PyModule_AddType(new_module, SystemCamera::GetPyType());
  PyModule_AddType(new_module, SystemModel::GetPyType());
  PyModule_AddType(new_module, SystemSyncRender::GetPyType());
  PyModule_AddType(new_module, SystemInput::GetPyType());

  ECS::InitFuncModule(new_module);

  auto math_module = PyImport_AddModule("_math");
  ECS::InitMathModule(math_module);
}

void InitPythonPost() {
  PyRun_SimpleString(
    "import script_main\n"
    "script_main.__start__()\n"
  );
}

void TickPython()
{
  PyRun_SimpleString(
    "import script_main\n"
    "script_main.tick()\n"
  );
}

}
