#include "system_scene.h"

#include "component_physics.h"
#include "entity_base.h"

namespace ECS {
class SystemPhysics : public System {
public:
  SystemPhysics() 
    : System(SystemType_Physics)
    , _accumulator(0.0f)
    , _frame_rate(1.0 / 60.0f)
  {}

  void Tick(float dt) override;
  void Start() override;
  void Stop() override;

private:
  void LoadBody(Entity* ent);

  float _accumulator;
  float _frame_rate;
};
} // namespace ECS
