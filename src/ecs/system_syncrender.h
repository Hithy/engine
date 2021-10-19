#include "system_scene.h"
#include <cstdint>

namespace ECS {
class SystemSyncRender : public System {
public:
  SystemSyncRender() : System(SystemType_SyncRender) {}

  void Tick(float dt) override;
  void Start() override;
  void Stop() override;

private:
  void UpdateCameraTrans();
  void UpdateObjects();
  void UpdateLights();
};
} // namespace ECS
