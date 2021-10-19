#include "system_scene.h"
#include <cstdint>

namespace ECS {
class SystemCamera : public System {
public:
  SystemCamera() : System(SystemType_Camera) {}

  void Tick(float dt) override;
  void Start() override;
  void Stop() override;
};
} // namespace ECS
