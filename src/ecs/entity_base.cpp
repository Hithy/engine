#include "entity_base.h"

namespace ECS {

uint64_t GenEntityID() {
  static uint64_t id = 0;
  return ++id;
}

Entity::Entity() : _type(EntityType_Base), _id(GenEntityID()) {}

Entity::~Entity() {
  for (auto const &comp : _components) {
    delete comp.second;
  }
  _components.clear();
}

bool Entity::AddComponent(Component *comp) {
  if (!comp) {
    return false;
  }
  auto comp_type = comp->GetType();
  if (_components.count(comp_type)) {
    return false;
  }

  _components[comp_type] = comp;
  comp->SetEntity(this);
  return true;
}

Component *Entity::GetComponent(ComponentType type) {
  auto comp_itr = _components.find(type);
  if (comp_itr != _components.end()) {
    return comp_itr->second;
  }
  return nullptr;
}

std::vector<ComponentType> Entity::GetComponentTypes() {
  std::vector<ComponentType> res;

  for (auto const &comp : _components) {
    res.push_back(comp.first);
  }

  return res;
}

} // namespace ECS
