#include "ecs/world.h"
#include "ecs/component_camera.h"
#include "ecs/component_trans.h"
#include "ecs/component_light.h"
#include "ecs/entity_base.h"

#include "glm/glm.hpp"

ECS::Entity* CreateSunLight(const glm::vec3& pos) {
  ECS::Entity* ent = new ECS::Entity();
  glm::mat4 init_trans = glm::mat4(1.0f);
  init_trans = glm::translate(init_trans, pos);
  ent->AddComponent(new ECS::ComponentTransform(init_trans));

  ECS::LightParam light_param;
  light_param.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
  light_param.diffuse = glm::vec3(10.0f, 10.0f, 10.0f);
  light_param.specular = glm::vec3(10.0f, 10.0f, 10.0f);

  light_param.constant = 1.0f;
  light_param.linear = 0.009f;
  light_param.quadratic = 0.0032f;
  auto comp_light = new ECS::ComponentLight(ECS::LightType_Point);
  comp_light->SetLightParam(light_param);

  ent->AddComponent(comp_light);

  ent->SetRef(0);
  return ent;
}

ECS::Entity *CreatePointLight(const glm::vec3 &pos) {
  ECS::Entity* ent = new ECS::Entity();
  glm::mat4 init_trans = glm::mat4(1.0f);
  init_trans = glm::translate(init_trans, pos);
  ent->AddComponent(new ECS::ComponentTransform(init_trans));

  ECS::LightParam light_param;
  light_param.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
  light_param.diffuse = glm::vec3(0.0f, 0.8f, 0.0f);
  light_param.specular = glm::vec3(0.0f, 0.8f, 0.0f);

  light_param.constant = 1.0f;
  light_param.linear = 0.09f;
  light_param.quadratic = 0.032f;
  auto comp_light = new ECS::ComponentLight(ECS::LightType_Point);
  comp_light->SetLightParam(light_param);
  
  ent->AddComponent(comp_light);

  ent->SetRef(0);
  return ent;
}

ECS::Entity* CreateDirLight(const glm::vec3& forward) {
  ECS::Entity* ent = new ECS::Entity();

  glm::mat4 trans = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), forward * -1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
  trans = glm::inverse(trans);

  ent->AddComponent(new ECS::ComponentTransform(trans));

  ECS::LightParam light_param;
  light_param.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
  light_param.diffuse = glm::vec3(0.3f, 0.3f, 0.3f);
  light_param.specular = glm::vec3(0.1f, 0.1f, 0.1f);
  
  auto comp_light = new ECS::ComponentLight(ECS::LightType_Direction);
  comp_light->SetLightParam(light_param);

  ent->AddComponent(comp_light);

  ent->SetRef(0);
  return ent;
}

ECS::Entity* CreateAmbientLight(const glm::vec3& forward) {
  ECS::Entity* ent = new ECS::Entity();

  glm::mat4 trans = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), forward * -1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
  trans = glm::inverse(trans);

  ent->AddComponent(new ECS::ComponentTransform(trans));

  ECS::LightParam light_param;
  light_param.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
  light_param.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
  light_param.specular = glm::vec3(0.0f, 0.0f, 0.0f);

  auto comp_light = new ECS::ComponentLight(ECS::LightType_Direction);
  comp_light->SetLightParam(light_param);

  ent->AddComponent(comp_light);

  ent->SetRef(0);
  return ent;
}

int main() {
  auto& world = ECS::World::GetInstance();
  world.Init();

  world.Run();

  return 0;
}
