#pragma once

#include <glm/glm.hpp>

#include "component_base.h"

namespace ECS {
class ComponentCamera : public Component {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(ComponentCamera);
  ComponentCamera()
    : Component(ComponentType_Camera)
    , _fov(60.0f)
    , _pitch(0.0f)
    , _ratio(1920.0 / 1080.0)
    , _lock(false)
    , _yaw(0.0f)
  {};

  float GetYaw() const { return _yaw; }
  float GetPitch() const { return _pitch; }
  float GetFOV() const { return _fov; }

  void SetYaw(float val) { _yaw = val; }
  void SetPitch(float val) {
    _pitch = val;
    if (_pitch > 89.0f) {
      _pitch = 89.0f;
    }

    if (_pitch < -89.0f) {
      _pitch = -89.0f;
    }
  }
  void SetFOV(float val) { _fov = val; }
  void SetProjectionRatio(float ratio) { _ratio = ratio; }

  glm::mat4 GetView();
  glm::mat4 GetProjection();

  void Lock(bool enable) { _lock = enable; }
  bool IsLocked() { return _lock; }

private:
  float _yaw;
  float _pitch;
  float _fov;
  float _ratio;
  float _lock;
};
} // namespace ECS
