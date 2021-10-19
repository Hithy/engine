#include <chrono>
#include <GLFW/glfw3.h>

#include "system_camera.h"
#include "component_camera.h"
#include "component_trans.h"
#include "entity_base.h"
#include "world.h"

namespace ECS {
void SystemCamera::Tick(float dt) {
  static auto last_time = std::chrono::steady_clock::now();
  auto now_time = std::chrono::steady_clock::now();
  auto delta_time = std::chrono::duration<double>(now_time - last_time);
  last_time = now_time;

  auto &world = World::GetInstance();

  auto& input = world.ctx.input;
  auto delta_x = (input.move_delta_x * input.sensitivity);
  auto delta_y = (input.move_delta_y * input.sensitivity);

  bool WPressed = (input.W_Status == GLFW_PRESS);
  bool SPressed = (input.S_Status == GLFW_PRESS);
  bool APressed = (input.A_Status == GLFW_PRESS);
  bool DPressed = (input.D_Status == GLFW_PRESS);

  auto ents = _scene->GetEntitiesByType(ComponentType_Camera);

  for (auto &ent : ents) {
    auto base_ent = dynamic_cast<Entity*>(ent);
    auto comp_cam = dynamic_cast<ComponentCamera*>(base_ent->GetComponent(ComponentType_Camera));
    auto comp_trans = dynamic_cast<ComponentTransform*>(base_ent->GetComponent(ComponentType_Transform));

    // update yaw/pitch
    comp_cam->SetYaw(comp_cam->GetYaw() + delta_x);
    comp_cam->SetPitch(comp_cam->GetPitch() - delta_y);

    // update pos
    const float camera_speed = delta_time.count() * 10.0f;
    auto view = comp_cam->GetView();
    view = glm::inverse(view);

    auto forward = view * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    auto right = view * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    auto cam_pos = comp_trans->GetPosition();

    if (WPressed) {
      cam_pos += glm::vec3(forward * camera_speed);
    }
    if (SPressed) {
      cam_pos -= glm::vec3(forward * camera_speed);
    }
    if (APressed) {
      cam_pos -= glm::vec3(right * camera_speed);
    }
    if (DPressed) {
      cam_pos += glm::vec3(right * camera_speed);
    }
    comp_trans->SetPosition(cam_pos);
  }
}

void SystemCamera::Start() {
}

void SystemCamera::Stop() {
}
}
