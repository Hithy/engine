#include "system_scene.h"
#include <cstdint>

namespace ECS {
class SystemInput : public System {
public:
  SystemInput() : System(SystemType_Input) {}

  void Tick(float dt) override;
  void Start() override;
  void Stop() override;
};
} // namespace ECS
