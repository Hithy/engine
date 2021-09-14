#include "ecs/world.h"
#include "ecs/scene.h"
#include "ecs/component_camera.h"
#include "ecs/component_trans.h"
#include "ecs/component_model.h"
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

int main() {
  auto& world = ECS::World::GetInstance();

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

  new_scene->AddEntity(CreateModel("/home/yao/Documents/sgb/untitled.obj",
                                   glm::vec3(-3.0f, 0.0f, -5.0f),
                                   glm::vec3(0.01f, 0.01f, 0.01f)));

  new_scene->AddEntity(CreateModel("/home/yao/Documents/sgb/untitled.obj",
                                   glm::vec3(3.0f, 0.0f, -5.0f),
                                   glm::vec3(0.01f, 0.01f, 0.01f)));

  world.AddScene(new_scene);

  world.Init();
  world.Run();

  return 0;
}
