#include "component_light.h"

#include <glad/glad.h>

namespace ECS {
  ComponentLight::ComponentLight(LightType type)
    : Component(ComponentType_Light), 
    _type(type), 
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
}