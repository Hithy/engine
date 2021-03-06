#pragma once
#include "system_scene.h"

namespace ECS {
class SystemModel : public System {
public:
  SystemModel() : System(SystemType_Model) {}

  void Tick(float dt) override;
  void Start() override{};
  void Stop() override{};
};
} // namespace ECS
