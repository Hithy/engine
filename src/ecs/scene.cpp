#include <iostream>

#include "scene.h"
#include "world.h"

#include "component_camera.h"
#include "component_light.h"
#include "component_model.h"
#include "component_trans.h"
#include "entity_base.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/Model.h"
#include "render/Shader.h"

namespace ECS {

Scene::Scene() {
  _active_camera = 0;
  _global_shader = new Shader("shader/common.vert", "shader/common.frag");
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

  auto base_ent = dynamic_cast<Entity *>(ent);
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

void Scene::updateShaderVP() {
  auto &world = World::GetInstance();
  float ratio = (1.0f * world.ctx.window_width) / world.ctx.window_height;

  auto cam_itr = _entities.find(_active_camera);
  if (cam_itr == _entities.end()) {
    return;
  }
  auto cam_ent = dynamic_cast<Entity *>(cam_itr->second);
  auto cam_comp = dynamic_cast<ComponentCamera *>(
      cam_ent->GetComponent(ComponentType_Camera));
  auto view = cam_comp->GetView();
  auto projection =
      glm::perspective(glm::radians(cam_comp->GetFOV()), ratio, 0.1f, 100.0f);

  _global_shader->SetFM4("view", glm::value_ptr(view));
  _global_shader->SetFM4("projection", glm::value_ptr(projection));

  auto comp_trans = dynamic_cast<ComponentTransform *>(
      cam_ent->GetComponent(ComponentType_Transform));
  auto cam_pos = comp_trans->GetPosition();
  _global_shader->SetFV3("viewPos", glm::value_ptr(cam_pos));
}

static void setShaderLightParam(Shader *shader, LightType light_type,
                                const std::string &light_prefix,
                                const LightParam &light_param,
                                ComponentTransform *comp_trans) {
  shader->SetFV3((light_prefix + ".ambient").c_str(),
                         glm::value_ptr(light_param.ambient));
  shader->SetFV3((light_prefix + ".diffuse").c_str(),
                         glm::value_ptr(light_param.diffuse));
  shader->SetFV3((light_prefix + ".specular").c_str(),
                         glm::value_ptr(light_param.specular));

  if (light_type == LightType_Point) {
    shader->SetFV3((light_prefix + ".position").c_str(),
                           glm::value_ptr(comp_trans->GetPosition()));
    shader->SetFloat((light_prefix + ".constant").c_str(),
                             light_param.constant);
    shader->SetFloat((light_prefix + ".linear").c_str(),
                             light_param.linear);
    shader->SetFloat((light_prefix + ".quadratic").c_str(),
                             light_param.quadratic);
  } else if (light_type == LightType_Direction) {
    auto direction = comp_trans->GetRotation() * glm::vec3(0.0f, 0.0f, 1.0f);
    shader->SetFV3((light_prefix + ".direction").c_str(),
                           glm::value_ptr(direction));
  }
}

void Scene::updateShaderLight() {
  static std::unordered_map<LightType, std::string> light_shader_name = {
      {LightType_Point, "point_light"},
      {LightType_Direction, "direction_light"}};
  std::unordered_map<LightType, int> light_count;

  auto light_ents = GetEntitiesByType(ComponentType_Light);
  for (const auto &light_ent : light_ents) {
    auto ent = dynamic_cast<Entity *>(light_ent);
    auto comp_trans = dynamic_cast<ComponentTransform *>(
        ent->GetComponent(ComponentType_Transform));
    auto comp_light =
        dynamic_cast<ComponentLight *>(ent->GetComponent(ComponentType_Light));

    const auto &light_param = comp_light->GetLightParam();
    auto light_type = comp_light->GetType();
    int light_num = ++light_count[light_type];

    std::string light_prefix = light_shader_name[light_type] + "_list[" +
                               std::to_string(light_num) + "]";

    setShaderLightParam(_global_shader, light_type, light_prefix, light_param, comp_trans);
  }

  for (auto const &light_name_itr : light_shader_name) {
    std::string light_count_name = light_name_itr.second + "_count";
    _global_shader->SetInt(light_count_name.c_str(), light_count[light_name_itr.first]);
  }
}

void Scene::Render() {
  updateShaderVP();
  updateShaderLight();

  _global_shader->Use();
  auto model_ents = GetEntitiesByType(ComponentType_Model);
  for (const auto &model_ent : model_ents) {
    auto base_ent = dynamic_cast<Entity *>(model_ent);
    auto comp_trans = dynamic_cast<ComponentTransform *>(
        base_ent->GetComponent(ComponentType_Transform));
    auto comp_model = dynamic_cast<ComponentModel *>(
        base_ent->GetComponent(ComponentType_Model));

    auto trans = comp_trans->GetTransform();
    _global_shader->SetFM4("model", glm::value_ptr(trans));

    comp_model->Draw(_global_shader);
  }
}

std::vector<IEntity *> Scene::GetEntitiesByType(ComponentType type) {
  std::vector<IEntity *> res;
  for (auto const &ent : _entity_tag_list[type]) {
    res.push_back(ent.second);
  }

  return res;
}
} // namespace ECS
