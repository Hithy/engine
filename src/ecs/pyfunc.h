#pragma once

namespace ECS {

  class ComponentModel;
  class ComponentTransform;
  class ComponentCamera;
  class ComponentLight;
  class Entity;
  class Scene;
  class SystemCamera;
  class SystemModel;
  class SystemInput;
  class SystemSyncRender;
 
  Entity* CreateEntity();
  ComponentModel* CreateComponentModel(const char* path);
  ComponentTransform* CreateComponentTransform();
  ComponentCamera* CreateComponentCamera();
  ComponentLight* CreateComponentLight(int light_type);
  Scene* CreateScene();

  SystemCamera* CreateSystemCamera();
  SystemModel* CreateSystemModel();
  SystemInput* CreateSystemInput();
  SystemSyncRender* CreateSystemSyncRender();

  void InitFuncModule(void* mod);
}