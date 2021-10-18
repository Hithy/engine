#pragma once
#include <string>

#include "component_base.h"

#include "pybind/pybind.h"
#include "ecs/system_model.h"

namespace render {
  class Model;
  class Shader;
}

namespace ECS {
class ComponentModel : public Component {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(ComponentModel);
  ComponentModel(const char *file_path);
  virtual ~ComponentModel();

  bool IsLoaded() const { return _loaded; }
  void SetLoaded() { _loaded = true; }

  void Draw(render::Shader* shader);

  void SetModelPath(const char* path) { _model_path = path; }
  void SetAlbedoPath(const char* path) { _albedo_path = path; }
  void SetNormalPath(const char* path) { _normal_path = path; }
  void SetMetalicPath(const char* path) { _metalic_path = path; }
  void SetRouphnessPath(const char* path) { _roughness_path = path; }
  void SetAOPath(const char* path) { _ao_path = path; }
  std::string GetModelPath() { return _model_path; }
  std::string GetAlbedoPath() { return _albedo_path; }
  std::string GetNormalPath() { return _normal_path; }
  std::string GetMetalicPath() { return _metalic_path; }
  std::string GetRouphnessPath() { return _roughness_path; }
  std::string GetAOPath() { return _ao_path; }

private:
  std::string _path;
  bool _loaded;
  render::Model* _model;

protected:
  std::string _model_path;
  std::string _albedo_path;
  std::string _normal_path;
  std::string _metalic_path;
  std::string _roughness_path;
  std::string _ao_path;

public:
  uint64_t model_id;
  uint64_t albedo_id;
  uint64_t normal_id;
  uint64_t metalic_id;
  uint64_t roughness_id;
  uint64_t ao_id;

  friend SystemModel;
};
} // namespace ECS
