#include "ecs/world.h"
#include "ecs/scene.h"
#include "ecs/component_camera.h"
#include "ecs/component_trans.h"
#include "ecs/component_model.h"
#include "ecs/component_light.h"
#include "ecs/entity_base.h"
#include "ecs/system_camera.h"
#include "ecs/system_model.h"
#include "ecs/system_input.h"

#include "glm/glm.hpp"

ECS::Entity *CreateModelObj(const char *path, const glm::vec3 &pos, const glm::vec3 scale) {
  ECS::Entity* model_ent = new ECS::Entity();
  glm::mat4 init_trans = glm::mat4(1.0f);
  init_trans = glm::translate(init_trans, pos);
  init_trans = glm::scale(init_trans, scale);
  model_ent->AddComponent(new ECS::ComponentTransform(init_trans));
  model_ent->AddComponent(new ECS::ComponentModel(path));
  model_ent->SetRef(0);

  return model_ent;
}

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

ECS::Entity* CreateCamera() {
  auto ent = new ECS::Entity();
  ent->AddComponent(new ECS::ComponentCamera());
  ent->AddComponent(new ECS::ComponentTransform());

  ent->SetRef(0);
  return ent;
}

int main() {
  auto& world = ECS::World::GetInstance();
  world.Init();

  // init system
  auto* scene_obj = new ECS::Scene();
  scene_obj->AddSystem(new ECS::SystemCamera());
  scene_obj->AddSystem(new ECS::SystemModel());
  scene_obj->AddSystem(new ECS::SystemInput());

  // create entity
  auto cam_ent = CreateCamera();
  scene_obj->AddEntity(cam_ent);
  // floor
  scene_obj->AddEntity(CreateModelObj("C:\\Users\\huyao\\Documents\\Models\\floor\\untitled.obj",
    glm::vec3(0.0f, -3.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f)));

  scene_obj->AddEntity(CreateModelObj("C:\\Users\\huyao\\Documents\\Models\\wall\\untitled.obj",
    glm::vec3(5.0f, 0.0f, -15.0f),
    glm::vec3(1.0f, 2.0f, 4.0f)));

  scene_obj->AddEntity(CreateModelObj("C:\\Users\\huyao\\Documents\\Models\\wall\\untitled.obj",
    glm::vec3(-5.0f, 0.0f, -15.0f),
    glm::vec3(1.0f, 2.0f, 4.0f)));

  scene_obj->AddEntity(CreateAmbientLight(glm::vec3(4.0f, -4.0f, 0.0f)));

  // scene_obj->AddEntity(CreateDirLight(glm::vec3(4.0f, -4.0f, 0.0f)));
  // scene_obj->AddEntity(CreateDirLight(glm::vec3(-4.0f, -4.0f, 4.0f)));

  scene_obj->AddEntity(CreatePointLight(glm::vec3(0.0f, 0.0f, -18.0f)));
  scene_obj->AddEntity(CreateSunLight(glm::vec3(0.0f, 5.0f, -35.0f)));
  // scene_obj->AddEntity(CreatePointLight(glm::vec3(5.0f, 5.0f, -20.0f)));
  /*scene_obj->AddEntity(CreateModelObj("C:\\Users\\huyao\\Documents\\Models\\sgb\\untitled.obj",
                                   glm::vec3(0.0f, 0.0f, -15.0f),
                                   glm::vec3(0.01f, 0.01f, 0.01f)));*/

  scene_obj->SetActiveCamera(cam_ent->GetID());

  world.AddScene(scene_obj);
  world.Run();

  return 0;
}
