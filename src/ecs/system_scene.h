#pragma once
#include "system.h"

#include "pybind/pyobject.h"

namespace ECS {
class Scene;

class System : public ISystem, public BindObject {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(System)
  System(int type) : _scene(nullptr), _type(static_cast<SystemType>(type)) {}

  virtual void Tick(float dt) {};
  virtual void Start() {};
  virtual void Stop() {};

  void ScriptTick(float dt) {
    Tick(dt);
  }

  void ScriptStart() {
    Start();
  }

  void ScriptStop() {
    Stop();
  }

  SystemType GetSystemType() const { return _type; }
  void SetScene(Scene* scn) { _scene = scn; }
  Scene* GetScene();

protected:
  Scene *_scene;

private:
  SystemType _type;
};
} // namespace ECS
