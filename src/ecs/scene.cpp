#include "scene.h"
#include <iostream>

#include "scene.h"
#include "world.h"

#include "component_camera.h"
#include "component_light.h"
#include "component_model.h"
#include "component_trans.h"
#include "entity_base.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "render/Model.h"
#include "render/Shader.h"
#include "render/resource.h"
#include "render/resource_mgr.h"
#include "render/render.h"

#include "ecs/utils.h"

#include "pybind/pybind.h"

namespace ECS {

Scene::Scene() : _active_camera(0), _ibl_hdr_path("") {
}

Scene::~Scene() {
  for (auto const &ent : _entities) {
    auto base_ent = dynamic_cast<Entity*>(ent.second);
    base_ent->DecRef();
  }
  _entities.clear();
  _entity_tag_list->clear();

  for (auto const &sys : _systems) {
    sys.second->DecRef();
  }
  _systems.clear();
}

void Scene::OnAddedToWorld()
{
  AddRef();
  std::cout << "scen added: " << this << std::endl;
}

bool Scene::AddSystem(System *sys) {
  if (!sys) {
    return false;
  }

  auto sys_type = sys->GetSystemType();
  if (_systems.count(sys_type)) {
    return false;
  }

  sys->SetScene(this);
  _systems[sys_type] = sys;
  sys->AddRef();

  return true;
}

bool Scene::DelSystem(SystemType sys_type) {
  auto sys_itr = _systems.find(sys_type);
  if (sys_itr == _systems.end()) {
    return false;
  }

  auto sys = sys_itr->second;
  _systems.erase(sys_itr);
  sys->DecRef();

  return true;
}

bool Scene::AddEntity(Entity* base_ent) {
  if (!base_ent) {
    return false;
  }

  uint64_t ent_id = base_ent->GetID();
  if (_entities.count(ent_id)) {
    return false;
  }

  base_ent->AddRef();
  _entities[ent_id] = base_ent;

  auto comp_types = base_ent->GetComponentTypes();
  for (auto const& comp_type : comp_types) {
    _entity_tag_list[comp_type][ent_id] = base_ent;
  }

  return true;
}

bool Scene::DelEntity(uint64_t ent_id) {
  auto ent_itr = _entities.find(ent_id);
  if (ent_itr == _entities.end()) {
    return false;
  }

  auto ent = ent_itr->second;
  _entities.erase(ent_id);
  for (auto &tag_list : _entity_tag_list) {
    tag_list.erase(ent_id);
  }
  auto base_ent = dynamic_cast<Entity*>(ent);
  base_ent->DecRef();

  return true;
}

decltype(auto) Scene::GetEntityCount()
{
  return _entities.size();
}

std::vector<uint64_t> Scene::GetEntityIds()
{
  std::vector<uint64_t> res;
  for (const auto& ent_itr : _entities) {
    res.push_back(ent_itr.first);
  }
  return res;
}

void Scene::Logic() {
  
}

std::vector<IEntity *> Scene::GetEntitiesByType(ComponentType type) {
  std::vector<IEntity *> res;
  for (auto const &ent : _entity_tag_list[type]) {
    res.push_back(ent.second);
  }

  return res;
}

Entity* Scene::GetEntitiesById(uint64_t ent_id)
{
  auto ent_itr = _entities.find(ent_id);
  if (ent_itr != _entities.end()) {
    return dynamic_cast<Entity*>(ent_itr->second);
  }
  return nullptr;
}

std::vector<Entity*> Scene::GetEntitiesByTypeExt(int type)
{
  std::vector<Entity*> res;
  for (auto const& ent : _entity_tag_list[type]) {
    res.push_back(dynamic_cast<Entity*>(ent.second));
  }

  return res;
}

std::vector<System*> Scene::GetSystems()
{
  std::vector<System*> res;
  for (int i = SystemType_Unknown + 1; i < SystemType_MAX; i++) {
    auto idx = static_cast<SystemType>(i);
    if (_systems.count(idx)) {
      res.push_back(_systems[idx]);
    }
  }
  return res;
}

BIND_CLS_FUNC_DEFINE(Scene, GetEntityCount)
BIND_CLS_FUNC_DEFINE(Scene, GetEntityIds)
BIND_CLS_FUNC_DEFINE(Scene, AddEntity)
BIND_CLS_FUNC_DEFINE(Scene, AddSystem)
BIND_CLS_FUNC_DEFINE(Scene, GetEntitiesById)
BIND_CLS_FUNC_DEFINE(Scene, SetActiveCamera)
BIND_CLS_FUNC_DEFINE(Scene, SetIBLPath)
BIND_CLS_FUNC_DEFINE(Scene, GetEntitiesByTypeExt)
BIND_CLS_FUNC_DEFINE(Scene, GetSystems)

static PyMethodDef type_methods[] = {
  {"get_entity_count", BIND_CLS_FUNC_NAME(Scene, GetEntityCount), METH_NOARGS, 0},
  {"GetEntityIds", BIND_CLS_FUNC_NAME(Scene, GetEntityIds), METH_NOARGS, 0},
  {"AddEntity", BIND_CLS_FUNC_NAME(Scene, AddEntity), METH_VARARGS, 0},
  {"AddSystem", BIND_CLS_FUNC_NAME(Scene, AddSystem), METH_VARARGS, 0},
  {"GetEntitiesById", BIND_CLS_FUNC_NAME(Scene, GetEntitiesById), METH_VARARGS, 0},
  {"SetActiveCamera", BIND_CLS_FUNC_NAME(Scene, SetActiveCamera), METH_VARARGS, 0},
  {"SetIBLPath", BIND_CLS_FUNC_NAME(Scene, SetIBLPath), METH_VARARGS, 0},
  {"GetEntitiesByType", BIND_CLS_FUNC_NAME(Scene, GetEntitiesByTypeExt), METH_VARARGS, 0},
  {"GetSystems", BIND_CLS_FUNC_NAME(Scene, GetSystems), METH_NOARGS, 0},
  {0, nullptr, 0, 0},
}; 

DEFINE_PYCXX_OBJECT_TYPE_BASE(Scene, "Scene", type_methods, py_init_params<>())
} // namespace ECS
