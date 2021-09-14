#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "entity.h"
#include "component.h"
#include "system_scene.h"

namespace ECS {
class Scene {
public:
  Scene();
  virtual ~Scene();

  void Logic();
  void Render();

  bool AddSystem(System* sys);
  bool DelSystem(SystemType sys_type);

  bool AddEntity(IEntity* ent);
  bool DelEntity(uint64_t ent_id);

  std::vector<IEntity*> GetEntitiesByType(ComponentType type);

  void SetActiveCamera(uint64_t ent_id) { _active_camera = ent_id; }

private:
  // id -> entity
  std::unordered_map<uint64_t, IEntity*> _entities;
  std::unordered_map<uint64_t, IEntity*> _entity_tag_list[ComponentType_MAX];

  std::unordered_map<SystemType, System*> _systems;

  uint64_t _active_camera;
};
} // namespace ECS
