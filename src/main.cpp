#include "ecs/world.h"

int main() {
  auto& world = ECS::World::GetInstance();
  world.Init();

  world.Run();

  return 0;
}
