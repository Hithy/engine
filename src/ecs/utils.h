#pragma once
#include <string>
#include <glm/glm.hpp>

#include "ecs/entity_base.h"
#include "ecs/component_light.h"
#include "ecs/component_trans.h"

namespace ECS {
  inline glm::mat4 GetDirectionLightMatrixVP(Entity* ent) {
    auto comp_trans = dynamic_cast<ComponentTransform*>(ent->GetComponent(ComponentType_Transform));

    auto light_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
      comp_trans->GetRotation() * glm::vec3(0.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, 1.0f, 0.0f));

    float near_plane = -100.0f, far_plane = 100.0f;
    auto light_proj = glm::ortho(-40.0f, 40.0f, -100.0f, 100.0f, near_plane, far_plane);
    
    return light_proj * light_view;
  }

}