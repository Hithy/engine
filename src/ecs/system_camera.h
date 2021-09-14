#include "system_scene.h"
#include <cstdint>

namespace ECS {
class SystemCamera : public System {
public:
  SystemCamera() : System(SystemType_Camera) {}

  void Tick() override;
  void Start() override;
  void Stop() override;

private:
  uint64_t _current;
};
} // namespace ECS
