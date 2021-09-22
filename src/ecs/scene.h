#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "component.h"
#include "entity.h"
#include "system_scene.h"

class Shader;

namespace ECS {
class Scene {
public:
  Scene();
  virtual ~Scene();

  void ToggleHDR();
  void ToggleGamma();

  void OnAddedToWorld();

  void Logic();
  void Render();

  bool AddSystem(System *sys);
  bool DelSystem(SystemType sys_type);

  bool AddEntity(IEntity *ent);
  bool DelEntity(uint64_t ent_id);

  std::vector<IEntity *> GetEntitiesByType(ComponentType type);

  void SetActiveCamera(uint64_t ent_id) { _active_camera = ent_id; }

private:
  void initHDR();

private:
  void updateShaderVP();
  void updateShaderLight();

  void drawObjects(Shader* shader);

  void renderScene();
  void renderShadow();
  void renderQuad();

private:
  // id -> entity
  std::unordered_map<uint64_t, IEntity *> _entities;
  std::unordered_map<uint64_t, IEntity *> _entity_tag_list[ComponentType_MAX];

  std::unordered_map<SystemType, System *> _systems;

  uint64_t _active_camera;
  Shader *_global_shader;

  bool _enable_hdr;
  bool _enable_gamma;

  unsigned int _shadow_fbo;
  unsigned int _hdr_fbo;
  unsigned int _hdr_vao;
  unsigned int _post_texture;
  unsigned int _post_depth;
};
} // namespace ECS
