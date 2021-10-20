#pragma once

namespace ECS {

enum SystemType {
  SystemType_Unknown = 0,
  SystemType_Input,
  SystemType_Camera,
  SystemType_Model,
  SystemType_Rotate,

  SystemType_SyncRender,
  SystemType_MAX,
};

class ISystem {
public:
  virtual SystemType GetSystemType() const = 0;
  virtual void Tick(float dt) = 0;
  virtual void Start() = 0;
  virtual void Stop() = 0;

  virtual ~ISystem() {};
};
}
