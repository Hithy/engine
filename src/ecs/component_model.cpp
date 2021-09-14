#include "component_model.h"

#include "render/Model.h"

namespace ECS {
void ComponentModel::Load() {
  _model = new Model(_path.c_str());
  _loaded = true;
}

void ComponentModel::Draw(Shader *shader) {
  _model->Draw(shader);
}

ComponentModel::~ComponentModel() {
  if (_model) {
    delete _model;
  }
}
}
