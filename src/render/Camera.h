#pragma once

#include <glm/glm.hpp>

class Camera {
public:
  Camera();

  glm::mat4 GetView();

  // private:
  glm::vec3 pos;
  glm::vec3 forward;
  glm::vec3 up;

  float yaw;
  float pitch;

  float fov;
};

