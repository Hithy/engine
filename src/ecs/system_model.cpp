#include "system_model.h"
#include "scene.h"
#include "entity_base.h"
#include "component_model.h"
#include "component_trans.h"

namespace ECS {

void SystemModel::Tick() {
  auto ents = _scene->GetEntitiesByType(ComponentType_Model);

  for (auto &ent : ents) {
    auto base_ent = dynamic_cast<Entity*>(ent);
    auto comp_trans = dynamic_cast<ComponentTransform*>(base_ent->GetComponent(ComponentType_Transform));
    auto comp_model = dynamic_cast<ComponentModel*>(base_ent->GetComponent(ComponentType_Model));

    if (!comp_model->IsLoaded()) {
      comp_model->Load();
    }
  }
}
}
