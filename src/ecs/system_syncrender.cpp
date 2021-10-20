#include "system_syncrender.h"
#include "world.h"
#include "component_camera.h"
#include "component_light.h"
#include "component_model.h"
#include "component_trans.h"
#include "entity_base.h"

#include "render/render.h"
#include "render/resource_mgr.h"

namespace ECS {
  void SystemSyncRender::Tick(float dt)
  {
    UpdateCameraTrans();
    UpdateLights();
    UpdateObjects();
  }
  void SystemSyncRender::Start()
  {
    auto& render = render::Render::GetInstance();

    // render.SetPbrSkyBox("resource/images/Chelsea_Stairs/Chelsea_Stairs_3k.hdr");
    render.SetPbrSkyBox("resource/images/hdr/newport_loft.hdr");
    render.PrepareRender();
  }
  void SystemSyncRender::Stop()
  {
  }
  void SystemSyncRender::UpdateCameraTrans()
  {
    auto cam_ent = dynamic_cast<Entity*>(_scene->GetEntitiesById(_scene->GetActiveCamera()));
    if (cam_ent) {
      auto& world = World::GetInstance();
      float ratio = (1.0f * world.ctx.window_width) / world.ctx.window_height;

      auto cam_comp = dynamic_cast<ComponentCamera*>(
        cam_ent->GetComponent(ComponentType_Camera));
      auto trans_comp = dynamic_cast<ComponentTransform*>(
        cam_ent->GetComponent(ComponentType_Transform));
      auto view = cam_comp->GetView();
      auto projection =
        glm::perspective(glm::radians(cam_comp->GetFOV()), ratio, 0.1f, 1000.0f);

      auto& render = render::Render::GetInstance();
      render.SetCameraTrans(view, projection, trans_comp->GetPosition());
    }
  }

  void SystemSyncRender::UpdateObjects()
  {
    auto& render = render::Render::GetInstance();
    render.ClearRenderItem();

    auto model_ents = _scene->GetEntitiesByType(ComponentType_Model);
    for (const auto& model_ent : model_ents) {
      auto base_ent = dynamic_cast<Entity*>(model_ent);
      auto comp_trans = dynamic_cast<ComponentTransform*>(
        base_ent->GetComponent(ComponentType_Transform));
      auto comp_model = dynamic_cast<ComponentModel*>(
        base_ent->GetComponent(ComponentType_Model));

      render::RenderItem item;
      item.obj_id = model_ent->GetID();
      item.transform = comp_trans->GetTransform();
      item.mesh = comp_model->model_id;
      item.albedo = comp_model->albedo_id;
      item.normal = comp_model->normal_id;
      item.metalic = comp_model->metalic_id;
      item.roughness = comp_model->roughness_id;
      item.ao = comp_model->ao_id;
      render.AddRenderItem(item);
    }

  }
  void SystemSyncRender::UpdateLights()
  {
    auto& render = render::Render::GetInstance();
    render.ClearPointLight();
    render.ClearDirectionLight();

    auto light_ents = _scene->GetEntitiesByType(ComponentType_Light);
    for (const auto& light_ent : light_ents) {
      auto base_ent = dynamic_cast<Entity*>(light_ent);
      auto comp_trans = dynamic_cast<ComponentTransform*>(
        base_ent->GetComponent(ComponentType_Transform));
      auto comp_light = dynamic_cast<ComponentLight*>(
        base_ent->GetComponent(ComponentType_Light));

      if (comp_light->GetLightType() == LightType_Point) {
        render::RenderPointLight point_light;

        point_light.light_id = base_ent->GetID();
        point_light.position = comp_trans->GetPosition();
        point_light.color = comp_light->GetLightParam().diffuse;

        render.AddPointLight(point_light);
      } else if (comp_light->GetLightType() == LightType_Direction) {
        render::RenderDirectionLight direction_light;

        direction_light.light_id = base_ent->GetID();
        direction_light.direction = glm::mat3(comp_trans->GetTransform()) * glm::vec3(0.0f, 0.0f, 1.0f);
        direction_light.color = comp_light->GetLightParam().diffuse;

        render.AddDirectionLight(direction_light);
      }
    }
  }
} // namespace ECS
