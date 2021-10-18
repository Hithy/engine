#pragma once
#include <memory>

#include "resource.h"

namespace render {
  std::shared_ptr<ResourceTexture2D> GetTexture2DResource(uint64_t id, bool auto_load = true);
  std::shared_ptr<ResourceTextureCube> GetTextureCubeResource(uint64_t id, bool auto_load = true);
  std::shared_ptr<ResourceModel> GetModelResource(uint64_t id, bool auto_load = true);
}