#include "entity_base.h"
#include "pybind/pybind.h"

namespace ECS {

uint64_t GenEntityID() {
  static uint64_t id = 0;
  return ++id;
}

Entity::Entity() : _type(EntityType_Base), _id(GenEntityID()) {}

Entity::~Entity() {
  for (auto const &comp : _components) {
    comp.second->DecRef();
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
  comp->AddRef();
  comp->SetEntity(this);
  return true;
}

Component *Entity::GetComponent(int type) {
  auto comp_itr = _components.find(static_cast<ComponentType>(type));
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

BIND_CLS_FUNC_DEFINE(Entity, GetID)
BIND_CLS_FUNC_DEFINE(Entity, AddComponent)
BIND_CLS_FUNC_DEFINE(Entity, GetComponent)

static PyMethodDef type_methods[] = {
    {"get_id", BIND_CLS_FUNC_NAME(Entity, GetID), METH_NOARGS, NULL},
    {"AddComponent", BIND_CLS_FUNC_NAME(Entity, AddComponent), METH_VARARGS, NULL},
    {"GetComponent", BIND_CLS_FUNC_NAME(Entity, GetComponent), METH_VARARGS, NULL},
    {0, nullptr, 0, 0},
};

DEFINE_PYCXX_OBJECT_TYPE_BASE(Entity, "Entity", type_methods)

} // namespace ECS
