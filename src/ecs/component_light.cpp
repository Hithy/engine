#include "component_light.h"

#include <glad/glad.h>
#include "pybind/pybind.h"

namespace ECS {
  ComponentLight::ComponentLight(int type)
    : Component(ComponentType_Light), 
    _type(static_cast<LightType>(type)), 
    _light(), 
    _shadow_texture(0)
  {
  }
  ComponentLight::~ComponentLight()
  {
    if (!_shadow_texture) {
      glDeleteTextures(1, &_shadow_texture);
    }
  }
  void ComponentLight::SetShadowTexture(unsigned int texture)
  {
    if (!_shadow_texture) {
      glDeleteTextures(1, &_shadow_texture);
    }
    _shadow_texture = texture;
  }

  void ComponentLight::SetLightColor(const glm::vec3& color)
  {
    _light.diffuse = color;
  }

  BIND_CLS_FUNC_DEFINE(ComponentLight, SetLightColor);

  static PyMethodDef type_methods[] = {
  {"SetLightColor", BIND_CLS_FUNC_NAME(ComponentLight, SetLightColor), METH_VARARGS, 0},
  {0, nullptr, 0, 0},
  };

  DEFINE_PYCXX_OBJECT_TYPE(Component, ComponentLight, "ComponentLight", type_methods, py_init_params<int>())
}