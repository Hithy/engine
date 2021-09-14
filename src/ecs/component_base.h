#pragma once

#include "component.h"
#include "entity.h"

namespace ECS {
class Component : public IComponent {
public:
  Component() : _type(ComponentType_Unknown), _ent(nullptr){};
  Component(ComponentType type) : _type(type), _ent(nullptr){};

  ComponentType GetType() const override { return _type; }

  void SetEntity(IEntity *ent) { _ent = ent; }
  IEntity *GetEntity() { return _ent; }

private:
  ComponentType _type;
  IEntity *_ent;
};
}; // namespace ECS
