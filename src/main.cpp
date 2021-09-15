#include "ecs/world.h"
#include "ecs/scene.h"
#include "ecs/component_camera.h"
#include "ecs/component_trans.h"
#include "ecs/component_model.h"
#include "ecs/component_light.h"
#include "ecs/entity_base.h"
#include "ecs/system_camera.h"
#include "ecs/system_model.h"

#include "glm/glm.hpp"

ECS::Entity *CreateModel(const char *path, const glm::vec3 &pos, const glm::vec3 scale) {
  ECS::Entity* model_ent = new ECS::Entity();
  glm::mat4 init_trans = glm::mat4(1.0f);
  init_trans = glm::translate(init_trans, pos);
  init_trans = glm::scale(init_trans, scale);
  model_ent->AddComponent(new ECS::ComponentTransform(init_trans));
  model_ent->AddComponent(new ECS::ComponentModel(path));

  return model_ent;
}

ECS::Entity *CreatePointLight(const glm::vec3 &pos) {
  ECS::Entity* ent = new ECS::Entity();
  glm::mat4 init_trans = glm::mat4(1.0f);
  init_trans = glm::translate(init_trans, pos);
  ent->AddComponent(new ECS::ComponentTransform(init_trans));

  ECS::LightParam light_param;
  light_param.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
  light_param.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
  light_param.specular = glm::vec3(1.0f, 1.0f, 1.0f);

  light_param.constant = 1.0f;
  light_param.linear = 0.09f;
  light_param.quadratic = 0.032f;
  auto comp_light = new ECS::ComponentLight(ECS::LightType_Point);
  comp_light->SetLightParam(light_param);
  
  ent->AddComponent(comp_light);
  return ent;
}

int main() {
  auto& world = ECS::World::GetInstance();
  world.Init();

  auto* new_scene = new ECS::Scene();
  new_scene->AddSystem(new ECS::SystemCamera());
  new_scene->AddSystem(new ECS::SystemModel());

  auto cam_ent = new ECS::Entity();
  cam_ent->AddComponent(new ECS::ComponentCamera());
  cam_ent->AddComponent(new ECS::ComponentTransform());
  new_scene->AddEntity(cam_ent);
  new_scene->SetActiveCamera(cam_ent->GetID());

  glm::mat4 init_trans;
  ECS::Entity* model_ent;

  new_scene->AddEntity(CreatePointLight(glm::vec3(0.0f, 5.0f, 0.0f)));

  new_scene->AddEntity(CreateModel("/home/yao/Documents/sgb/untitled.obj",
                                   glm::vec3(0.0f, 0.0f, -5.0f),
                                   glm::vec3(0.01f, 0.01f, 0.01f)));

  new_scene->AddEntity(CreateModel("/home/yao/Documents/floor/untitled.obj",
                                   glm::vec3(0.0f, -3.0f, 0.0f),
                                   glm::vec3(1.0f, 1.0f, 1.0f)));

  

  world.AddScene(new_scene);
  
  world.Run();

  return 0;
}
