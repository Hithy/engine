#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "component.h"
#include "entity_base.h"
#include "system_scene.h"
#include "pybind/pyobject.h"

namespace ECS {
class Scene : public BindObject {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(Scene);
  Scene();
  virtual ~Scene();

  void OnAddedToWorld();
  void Logic();

  bool AddSystem(System *sys);
  bool DelSystem(SystemType sys_type);

  bool AddEntity(Entity *ent);
  bool DelEntity(uint64_t ent_id);

  decltype(auto) GetEntityCount();
  std::vector<uint64_t> GetEntityIds();

  std::vector<IEntity*> GetEntitiesByType(ComponentType type);
  Entity* GetEntitiesById(uint64_t ent_id);

  std::vector<Entity*> GetEntitiesByTypeExt(int type);
  std::vector<System*> GetSystems();

  void SetActiveCamera(uint64_t ent_id) { _active_camera = ent_id; }
  uint64_t GetActiveCamera() { return _active_camera; }

  const std::string& GetIBLHdrPath() { return _ibl_hdr_path; }
  void SetIBLPath(const char* hdr_path) { _ibl_hdr_path = hdr_path; }

private:
  // id -> entity
  std::unordered_map<uint64_t, IEntity *> _entities;
  std::unordered_map<uint64_t, IEntity *> _entity_tag_list[ComponentType_MAX];

  std::unordered_map<SystemType, System *> _systems;

  uint64_t _active_camera;
  std::string _ibl_hdr_path;
};
} // namespace ECS
