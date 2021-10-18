#include "system_model.h"
#include "scene.h"
#include "entity_base.h"
#include "component_model.h"
#include "component_trans.h"

#include "render/resource_mgr.h"

namespace ECS {

void SystemModel::Tick() {
  auto ents = _scene->GetEntitiesByType(ComponentType_Model);

  for (auto &ent : ents) {
    auto base_ent = dynamic_cast<Entity*>(ent);
    auto comp_trans = dynamic_cast<ComponentTransform*>(base_ent->GetComponent(ComponentType_Transform));
    auto comp_model = dynamic_cast<ComponentModel*>(base_ent->GetComponent(ComponentType_Model));

    if (!comp_model->IsLoaded()) {
      comp_model->model_id = render::GenModel(comp_model->_model_path.c_str());
      comp_model->albedo_id = render::GenTexture2DFromFile(comp_model->_albedo_path.c_str());
      comp_model->normal_id = render::GenTexture2DFromFile(comp_model->_normal_path.c_str());
      comp_model->metalic_id = render::GenTexture2DFromFile(comp_model->_metalic_path.c_str());
      comp_model->roughness_id = render::GenTexture2DFromFile(comp_model->_roughness_path.c_str());
      comp_model->ao_id = render::GenTexture2DFromFile(comp_model->_ao_path.c_str());
      comp_model->SetLoaded();
    }
  }
}
}
