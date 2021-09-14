#pragma once

namespace ECS {

enum ComponentType {
  ComponentType_Unknown = 0,
  ComponentType_Base,
  ComponentType_Transform,
  ComponentType_Camera,
  ComponentType_Model,
  ComponentType_Physics,

  ComponentType_MAX,
};

class IComponent {
public:
  virtual ComponentType GetType() const = 0;
  virtual ~IComponent(){};
};
} // namespace ECS
