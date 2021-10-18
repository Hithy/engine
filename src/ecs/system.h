#pragma once

namespace ECS {

enum SystemType {
  SystemType_Unknown = 0,
  SystemType_Camera,
  SystemType_Model,
  SystemType_Input,
  SystemType_SyncRender,
};

class ISystem {
public:
  virtual SystemType GetSystemType() const = 0;
  virtual void Tick() = 0;
  virtual void Start() = 0;
  virtual void Stop() = 0;

  virtual ~ISystem() {};
};
}
