#include "python_ecs.h"
#include "pybind/pybind.h"

#include "ecs/entity_base.h"
#include "ecs/world.h"
#include "ecs/scene.h"

#include <Python.h>
#include <vector>

#ifdef WIN32
#include <Windows.h>
#else
#endif

namespace ECS {
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
  GetAbsPath(output, 2048, "3rd\\Python-3.9.7");
  Py_SetPythonHome(output);

  std::wstring path = Py_GetPath();
  memset(output, 0, sizeof(output[0]) * 2048);
  GetAbsPath(output, 2048, "script");
  if (path.size()) {
    path += L";";
  }
  path += output;
  Py_SetPath(path.c_str());
}

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