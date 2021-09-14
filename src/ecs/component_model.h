#pragma once
#include <string>

#include "component_base.h"

class Model;
class Shader;

namespace ECS {
class ComponentModel : public Component {
public:
  ComponentModel(const char *file_path)
      : Component(ComponentType_Model), _path(file_path), _loaded(false), _model(nullptr){};
  virtual ~ComponentModel();

  void Load();
  bool IsLoaded() const { return _loaded; }

  void Draw(Shader* shader);

private:
  std::string _path;
  bool _loaded;
  Model* _model;
};
} // namespace ECS
