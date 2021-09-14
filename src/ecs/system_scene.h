#pragma once
#include "system.h"

namespace ECS {
class Scene;

class System : public ISystem {
public:
  System(SystemType type) : _scene(nullptr), _type(type) {}

  SystemType GetSystemType() const { return _type; }
  void SetScene(Scene *scn) { _scene = scn; }

protected:
  Scene *_scene;

private:
  SystemType _type;
};
} // namespace ECS
