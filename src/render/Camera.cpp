#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
  : pos(glm::vec3(0.0f)), forward(glm::vec3(0.0f, 0.0f, -1.0f)),
  up(glm::vec3(0.0f, 1.0f, 0.0f)), yaw(0.0f), pitch(0.0f), fov(60.0f) {
}

glm::mat4 Camera::GetView() {
  auto yaw_rot = glm::rotate(glm::mat4(1.0f), glm::radians(-yaw), glm::vec3(0.0f, 1.0f, 0.0f));
  auto new_right_vec = yaw_rot * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
  auto pitch_rot = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(new_right_vec));
  auto rotate_mat = glm::inverse(pitch_rot * yaw_rot);

  return rotate_mat * glm::lookAt(pos, pos + forward, up);
}
