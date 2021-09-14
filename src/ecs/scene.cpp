#include <iostream>

#include "scene.h"
#include "world.h"

#include "component_camera.h"
#include "component_trans.h"
#include "component_model.h"
#include "entity_base.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/Shader.h"
#include "render/Model.h"

namespace ECS {

Scene::Scene() {
  _active_camera = 0;
}

Scene::~Scene() {
  for (auto const &ent : _entities) {
    delete ent.second;
  }
  _entities.clear();
  _entity_tag_list->clear();

  for (auto const &sys : _systems) {
    delete sys.second;
  }
  _systems.clear();
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
  sys->Start();

  return true;
}

bool Scene::DelSystem(SystemType sys_type) {
  auto sys_itr = _systems.find(sys_type);
  if (sys_itr == _systems.end()) {
    return false;
  }

  auto sys = sys_itr->second;
  sys->Stop();
  _systems.erase(sys_itr);
  delete sys;

  return true;
}

bool Scene::AddEntity(IEntity *ent) {
  if (!ent) {
    return false;
  }

  uint64_t ent_id = ent->GetID();
  if (_entities.count(ent_id)) {
    return false;
  }

  _entities[ent_id] = ent;

  auto base_ent = dynamic_cast<Entity*>(ent);
  if (base_ent) {
    auto comp_types = base_ent->GetComponentTypes();
    for (auto const &comp_type : comp_types) {
      _entity_tag_list[comp_type][ent_id] = ent;
    }
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
  delete ent;

  return true;
}

void Scene::Logic() {
  for (auto const &sys : _systems) {
    sys.second->Tick();
  }
}

void Scene::Render() {
  static auto shader = new Shader("shader/common.vert", "shader/common.frag");

  auto cam_itr = _entities.find(_active_camera);
  if (cam_itr == _entities.end()) {
    return;
  }
  auto cam_ent = dynamic_cast<Entity*>(cam_itr->second);
  auto cam_comp = dynamic_cast<ComponentCamera*>(cam_ent->GetComponent(ComponentType_Camera));
  auto view = cam_comp->GetView();
  auto projection = glm::perspective(glm::radians(cam_comp->GetFOV()),
                                     1920.0f / 1080.0f, 0.1f, 100.0f);

  shader->Use();
  shader->SetFM4("view", glm::value_ptr(view));
  shader->SetFM4("projection", glm::value_ptr(projection));

  auto model_ents = GetEntitiesByType(ComponentType_Model);
  for (const auto &model_ent : model_ents) {
    auto base_ent = dynamic_cast<Entity*>(model_ent);
    auto comp_trans = dynamic_cast<ComponentTransform*>(base_ent->GetComponent(ComponentType_Transform));
    auto comp_model = dynamic_cast<ComponentModel*>(base_ent->GetComponent(ComponentType_Model));

    auto trans = comp_trans->GetTransform();
    shader->SetFM4("model", glm::value_ptr(trans));

    comp_model->Draw(shader);
  }
}

std::vector<IEntity *> Scene::GetEntitiesByType(ComponentType type) {
  std::vector<IEntity *> res;
  for (auto const &ent : _entity_tag_list[type]) {
    res.push_back(ent.second);
  }

  return res;
}
}
