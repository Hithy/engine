#include "resource_utils.h"
#include "resource_mgr.h"

namespace render {
  std::shared_ptr<ResourceTexture2D> GetTexture2DResource(uint64_t id, bool auto_load) {
    auto texture = ResourceMgr::GetInstance().GetResourceAs<ResourceTexture2D>(id);
    if (!texture->IsLoaded() && auto_load) {
      texture->Load();
    }
    return texture;
  }
  std::shared_ptr<ResourceTextureCube> GetTextureCubeResource(uint64_t id, bool auto_load) {
    auto texture = ResourceMgr::GetInstance().GetResourceAs<ResourceTextureCube>(id);
    if (!texture->IsLoaded() && auto_load) {
      texture->Load();
    }
    return texture;
  }
  std::shared_ptr<ResourceModel> GetModelResource(uint64_t id, bool auto_load)
  {
    auto model = ResourceMgr::GetInstance().GetResourceAs<ResourceModel>(id);
    if (!model->IsLoaded() && auto_load) {
      model->Load();
    }
    return model;
  }
}