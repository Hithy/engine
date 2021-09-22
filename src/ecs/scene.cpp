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

#include "ecs/utils.h"

namespace ECS {

Scene::Scene() : _active_camera(0), _global_shader(nullptr), _shadow_fbo(0), _enable_hdr(false), _enable_gamma(false) {
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

void Scene::ToggleHDR()
{
  _enable_hdr = !_enable_hdr;
  std::cout << (_enable_hdr ? "hdr enabled\n" : "hdr disabled\n");
}

void Scene::ToggleGamma()
{
  _enable_gamma = !_enable_gamma;
  std::cout << (_enable_gamma ? "gamma enabled\n" : "gamma disabled\n");
}

void Scene::OnAddedToWorld()
{
  _global_shader = new Shader("shader/common.vert", "shader/common.frag");
  glGenFramebuffers(1, &_shadow_fbo);

  initHDR();
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

void Scene::initHDR()
{
  auto const& world = World::GetInstance();

  glGenFramebuffers(1, &_hdr_fbo);
  glGenTextures(1, &_post_texture);
  glBindTexture(GL_TEXTURE_2D, _post_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, world.ctx.window_width, world.ctx.window_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenRenderbuffers(1, &_post_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, _post_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, world.ctx.window_width, world.ctx.window_height);
  glBindFramebuffer(GL_FRAMEBUFFER, _hdr_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _post_texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _post_depth);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  float quadVertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
  };
  // setup plane VAO
  unsigned int hdr_vbo;
  glGenVertexArrays(1, &_hdr_vao);
  glGenBuffers(1, &hdr_vbo);
  glBindVertexArray(_hdr_vao);
  glBindBuffer(GL_ARRAY_BUFFER, hdr_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
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
      glm::perspective(glm::radians(cam_comp->GetFOV()), ratio, 0.1f, 1000.0f);

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

static std::unordered_map<LightType, std::string> light_shader_name = {
        {LightType_Point, "point_light"},
       {LightType_Direction, "direction_light"} };

void Scene::updateShaderLight() {
  std::unordered_map<LightType, int> light_count;

  auto light_ents = GetEntitiesByType(ComponentType_Light);

  for (int j = 0; j < 5; j++) {
    std::string light_prefix = "point_light_list[" + std::to_string(j) + "].shadow_map";
    _global_shader->SetInt(light_prefix.c_str(), 5 + j);

    light_prefix = "direction_light_list[" + std::to_string(j) + "].shadow_map";
    _global_shader->SetInt(light_prefix.c_str(), 10 + j);
  }
  int i = 0;
  for (const auto &light_ent : light_ents) {
    auto ent = dynamic_cast<Entity *>(light_ent);
    auto comp_trans = dynamic_cast<ComponentTransform *>(
        ent->GetComponent(ComponentType_Transform));
    auto comp_light =
        dynamic_cast<ComponentLight *>(ent->GetComponent(ComponentType_Light));

    const auto &light_param = comp_light->GetLightParam();
    auto light_type = comp_light->GetLightType();
    int light_num = light_count[light_type]++;

    std::string light_prefix = light_shader_name[light_type] + "_list[" +
                               std::to_string(light_num) + "]";

    setShaderLightParam(_global_shader, light_type, light_prefix, light_param, comp_trans);

    if (comp_light->GetLightType() == LightType_Direction) {
      unsigned int shadow_texture = comp_light->GetShadowTexture();
      if (shadow_texture) {
        glActiveTexture(GL_TEXTURE10 + i);
        glBindTexture(GL_TEXTURE_2D, shadow_texture);
        auto shadow_vp = GetDirectionLightMatrixVP(ent);
        _global_shader->SetFM4((light_prefix + ".shadow_vp").c_str(), glm::value_ptr(shadow_vp));
        _global_shader->SetInt((light_prefix + ".shadow_map").c_str(), 10 + i);
        _global_shader->SetInt((light_prefix + ".enable_shadow").c_str(), 1);
      }
      else {
        _global_shader->SetInt((light_prefix + ".enable_shadow").c_str(), 0);
      }
    }
    else if (comp_light->GetLightType() == LightType_Point) {
      unsigned int shadow_texture = comp_light->GetShadowTexture();
      if (shadow_texture) {
        glActiveTexture(GL_TEXTURE5 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_texture);
        _global_shader->SetInt((light_prefix + ".shadow_map").c_str(), 5 + i);
        _global_shader->SetInt((light_prefix + ".enable_shadow").c_str(), 1);
      }
      else {
        _global_shader->SetInt((light_prefix + ".enable_shadow").c_str(), 0);
      }
    }

    i++;
  }

  for (auto const &light_name_itr : light_shader_name) {
    std::string light_count_name = light_name_itr.second + "_count";
    _global_shader->SetInt(light_count_name.c_str(), light_count[light_name_itr.first]);
  }
}

void Scene::drawObjects(Shader* shader)
{
  shader->Use();
  auto model_ents = GetEntitiesByType(ComponentType_Model);
  for (const auto& model_ent : model_ents) {
    auto base_ent = dynamic_cast<Entity*>(model_ent);
    auto comp_trans = dynamic_cast<ComponentTransform*>(
      base_ent->GetComponent(ComponentType_Transform));
    auto comp_model = dynamic_cast<ComponentModel*>(
      base_ent->GetComponent(ComponentType_Model));

    auto trans = comp_trans->GetTransform();
    shader->SetFM4("model", glm::value_ptr(trans));

    comp_model->Draw(shader);
  }
}

void Scene::renderScene()
{
  _global_shader->Use();
  
  glBindFramebuffer(GL_FRAMEBUFFER, _hdr_fbo);
  glViewport(0, 0, 1920, 1080);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  updateShaderVP();
  updateShaderLight();

  // _global_shader->Validate();

  drawObjects(_global_shader);
}

static const unsigned int SHADOW_WIDTH = 2048;
static const unsigned int SHADOW_HEIGHT = 2048;

static unsigned int GenDirectionShadowMap() {
  unsigned int res;
  glGenTextures(1, &res);
  glBindTexture(GL_TEXTURE_2D, res);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  glBindTexture(GL_TEXTURE_2D, 0);
  return res;
}

static void SetDirectionLightShadowParam(Entity* ent, Shader* shader) {
  auto comp_light = dynamic_cast<ComponentLight*>(ent->GetComponent(ComponentType_Light));
  auto shadow_vp = GetDirectionLightMatrixVP(ent);
  unsigned int shadow_texture = comp_light->GetShadowTexture();

  assert(comp_light->GetLightType() == LightType_Direction);

  if (!shadow_texture) {
    shadow_texture = GenDirectionShadowMap();
    comp_light->SetShadowTexture(shadow_texture);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_texture, 0);
  glClear(GL_DEPTH_BUFFER_BIT);

  shader->Use();
  shader->SetFM4("shadow_vp", glm::value_ptr(shadow_vp));
}

static unsigned int GenPointShadowMap() {
  unsigned int res;
  glGenTextures(1, &res);
  glActiveTexture(GL_TEXTURE0 + 10);
  glBindTexture(GL_TEXTURE_CUBE_MAP, res);
  for (int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
      SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  return res;
}

static void SetPointLightShadowParam(Entity* ent, Shader* shader) {
  auto comp_light = dynamic_cast<ComponentLight*>(ent->GetComponent(ComponentType_Light));
  auto comp_trans = dynamic_cast<ComponentTransform*>(ent->GetComponent(ComponentType_Transform));

  assert(comp_light->GetLightType() == LightType_Point);

  auto shadow_texture = comp_light->GetShadowTexture();
  if (!shadow_texture) {
    shadow_texture = GenPointShadowMap();
    comp_light->SetShadowTexture(shadow_texture);
  }

   glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture, 0);
   glClear(GL_DEPTH_BUFFER_BIT);

  float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
  float near = 1.0f;
  float far = 125.0f;
  glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
  auto lightPos = comp_trans->GetPosition();

  std::vector<glm::mat4> shadowTransforms;
  shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
  shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
  shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
  shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
  shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
  shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

  shader->Use();
  shader->SetFloat("far_plane", far);
  shader->SetFV3("lightPos", glm::value_ptr(lightPos));
  for (int i = 0; i < shadowTransforms.size(); i++) {
    std::string curr_vp = "shadowMatrices[" + std::to_string(i) + "]";
    shader->SetFM4(curr_vp.c_str(), glm::value_ptr(shadowTransforms[i]));
  }
}

void Scene::renderShadow()
{
  static Shader* shadow_shader_dir = new Shader("shader/shadow_vs.glsl", "shader/shadow_fg.glsl");
  static Shader* shadow_shader_point = new Shader("shader/shadow_point_vs.glsl", "shader/shadow_point_gs.glsl", "shader/shadow_point_fg.glsl");

  glBindFramebuffer(GL_FRAMEBUFFER, _shadow_fbo);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

  auto light_ents = GetEntitiesByType(ComponentType_Light);
  for (const auto& light_ent : light_ents) {
    auto base_ent = dynamic_cast<Entity*>(light_ent);
    auto comp_light = dynamic_cast<ComponentLight*>(base_ent->GetComponent(ComponentType_Light));

    switch (comp_light->GetLightType())
    {
    case LightType_Direction:
      SetDirectionLightShadowParam(base_ent, shadow_shader_dir);
      drawObjects(shadow_shader_dir);
      break;
    case LightType_Point:
      SetPointLightShadowParam(base_ent, shadow_shader_point);
      drawObjects(shadow_shader_point);
      break;
    default:
      break;
    }
  }
}

void Scene::renderQuad()
{
  static Shader* hdr_shader = new Shader("shader/hdr_vs.glsl", "shader/hdr_fg.glsl");

  hdr_shader->Use();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _post_texture);

  hdr_shader->SetInt("hdr", int(_enable_hdr));
  hdr_shader->SetInt("hdrBuffer", 0);
  hdr_shader->SetFloat("exposure", 1.0f);
  hdr_shader->SetFloat("gamma", _enable_gamma ? 2.2f : 1.0f);

  glBindVertexArray(_hdr_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void Scene::Render() {
  renderShadow();
  renderScene();
  renderQuad();
}

std::vector<IEntity *> Scene::GetEntitiesByType(ComponentType type) {
  std::vector<IEntity *> res;
  for (auto const &ent : _entity_tag_list[type]) {
    res.push_back(ent.second);
  }

  return res;
}
} // namespace ECS
