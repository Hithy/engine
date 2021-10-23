#include "component_model.h"

#include "render/Model.h"
#include "pybind/pybind.h"

namespace ECS {

void ComponentModel::Draw(render::Shader *shader) {
  _model->Draw(shader);
}

ComponentModel::ComponentModel(const char* file_path)
  : Component(ComponentType_Model)
  , _path(file_path)
  , _model_path(file_path)
  , _albedo_path("resource/images/pbr/white/albedo.png")
  , _normal_path("resource/images/pbr/white/normal.png")
  , _metalic_path("resource/images/pbr/white/metallic.png")
  , _roughness_path("resource/images/pbr/white/roughness.png")
  , _ao_path("resource/images/pbr/white/ao.png")
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

BIND_CLS_FUNC_DEFINE(ComponentModel, SetModelPath);
BIND_CLS_FUNC_DEFINE(ComponentModel, SetAlbedoPath);
BIND_CLS_FUNC_DEFINE(ComponentModel, SetNormalPath);
BIND_CLS_FUNC_DEFINE(ComponentModel, SetMetalicPath);
BIND_CLS_FUNC_DEFINE(ComponentModel, SetRouphnessPath);
BIND_CLS_FUNC_DEFINE(ComponentModel, SetAOPath);

static PyMethodDef type_methods[] = {
  {"SetModelPath", BIND_CLS_FUNC_NAME(ComponentModel, SetModelPath), METH_VARARGS, 0},
  {"SetAlbedoPath", BIND_CLS_FUNC_NAME(ComponentModel, SetAlbedoPath), METH_VARARGS, 0},
  {"SetNormalPath", BIND_CLS_FUNC_NAME(ComponentModel, SetNormalPath), METH_VARARGS, 0},
  {"SetMetalicPath", BIND_CLS_FUNC_NAME(ComponentModel, SetMetalicPath), METH_VARARGS, 0},
  {"SetRouphnessPath", BIND_CLS_FUNC_NAME(ComponentModel, SetRouphnessPath), METH_VARARGS, 0},
  {"SetAOPath", BIND_CLS_FUNC_NAME(ComponentModel, SetAOPath), METH_VARARGS, 0},
  {0, nullptr, 0, 0},
};

DEFINE_PYCXX_OBJECT_TYPE(Component, ComponentModel, "ComponentModel", type_methods, py_init_params<const char*>())
}
