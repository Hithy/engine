#include "component_model.h"

#include "render/Model.h"

namespace ECS {

void ComponentModel::Draw(render::Shader *shader) {
  _model->Draw(shader);
}

ComponentModel::ComponentModel(const char* file_path)
  : Component(ComponentType_Model)
  , _path(file_path)
  , _model_path(file_path)
  , _albedo_path("resource/images/pbr/gold/albedo.png")
  , _normal_path("resource/images/pbr/gold/normal.png")
  , _metalic_path("resource/images/pbr/gold/metallic.png")
  , _roughness_path("resource/images/pbr/gold/roughness.png")
  , _ao_path("resource/images/pbr/gold/ao.png")
  , model_id(0)
  , albedo_id(0)
  , normal_id(0)
  , metalic_id(0)
  , roughness_id(0)
  , ao_id(0)
  , _loaded(false)
  , _model(nullptr)
{
}

ComponentModel::~ComponentModel() {
  if (_model) {
    delete _model;
  }
}

DEFINE_PYCXX_OBJECT_TYPE_ENGINE(Component, ComponentModel, "ComponentModel", nullptr)
}
