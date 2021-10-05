#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "entity.h"
#include "component_base.h"
#include "pybind/pyobject.h"

namespace ECS {

class Entity : public IEntity, public PyCXXObject<Entity> {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(Entity);
  Entity();
  ~Entity();

  uint64_t GetID() override { return _id; }
  EntityType GetType() const override { return _type; }

  bool AddComponent(Component* comp);
  Component* GetComponent(ComponentType type);

  std::vector<ComponentType> GetComponentTypes();

private:
  EntityType _type;
  uint64_t _id;
  std::unordered_map<ComponentType, Component*> _components;
};

} // namespace ECS
