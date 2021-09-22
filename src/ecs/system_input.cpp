#include <GLFW/glfw3.h>

#include "system_input.h"
#include "world.h"

namespace ECS {
void SystemInput::Tick() {
  auto &world = World::GetInstance();
  auto& input = world.ctx.input;

  if (input.KeyToggled['H'] && input.KeyStatus['H'] == GLFW_PRESS) {
    _scene->ToggleHDR();
  }

  if (input.KeyToggled['G'] && input.KeyStatus['G'] == GLFW_PRESS) {
    _scene->ToggleGamma();
  }
}

void SystemInput::Start() {
}

void SystemInput::Stop() {
}
}
