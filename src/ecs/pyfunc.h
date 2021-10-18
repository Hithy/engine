#pragma once

namespace ECS {

  class ComponentModel;
  class ComponentTransform;
  class ComponentCamera;
  class ComponentLight;
  class Entity;
  class Scene;
 
  Entity* CreateEntity();
  ComponentModel* CreateComponentModel(const char* path);
  ComponentTransform* CreateComponentTransform();
  ComponentCamera* CreateComponentCamera();
  ComponentLight* CreateComponentLight(int light_type);
  Scene* CreateScene();

  void InitFuncModule(void* mod);
}