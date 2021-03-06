#pragma once
#include <string>

#include "component_base.h"
#include "glm/glm.hpp"

class Model;
class Shader;

namespace ECS {

struct LightParam {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;

  float constant;
  float linear;
  float quadratic;

  LightParam()
      : ambient(glm::vec3(0.0f, 0.0f, 0.0f)),
        diffuse(glm::vec3(0.0f, 0.0f, 0.0f)),
        specular(glm::vec3(0.0f, 0.0f, 0.0f)), constant(1.0f), linear(0.0f),
        quadratic(0.0f) {}
};

enum LightType {
  LightType_Unknown = 0,
  LightType_Direction,
  LightType_Point,
};

class ComponentLight : public Component {
public:
  DECLEAR_PYCXX_OBJECT_TYPE(ComponentLight);
  ComponentLight(int type);
  virtual ~ComponentLight();

  LightType GetLightType() { return _type; }

  void SetLightParam(const LightParam &light) { _light = light; }
  const LightParam &GetLightParam() const { return _light; }

  void SetShadowTexture(unsigned int texture);
  unsigned int GetShadowTexture() const { return _shadow_texture; }

  void SetLightColor(const glm::vec3& color);
  void SetEnableShadow(bool enable);
  bool IsShadowEnabled() { return _enable_shadow; }
  void SetRadius(float rad);
  float GetRadius() { return _radius; }

private:
  LightType _type;
  LightParam _light;
  bool _enable_shadow;
  float _radius;

  unsigned int _shadow_texture;
};
} // namespace ECS
