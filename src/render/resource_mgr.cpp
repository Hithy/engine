#include <unordered_map>
#include <string>

#include "resource_mgr.h"
#include "resource.h"

namespace render {

  Resource* GenResource(ResourceType type) {
    return nullptr;

  }

  uint64_t GenTexture2D(unsigned int width, unsigned int height, bool with_mipmap, int NR)
  {
    std::shared_ptr<Resource> new_texture(new ResourceTexture2D(width, height, with_mipmap, NR));
    ResourceMgr::GetInstance().SetResource(new_texture);
    return new_texture->GetID();
  }

  uint64_t GenTexture2DFromFile(const char* path, bool with_mipmap, bool is_hdr)
  {
    static std::unordered_map<std::string, uint64_t> cache;
    std::string cache_idx = path;
    if (!cache.count(cache_idx)) {
      std::shared_ptr<Resource> new_texture(new ResourceTexture2D(path, with_mipmap, is_hdr));
      ResourceMgr::GetInstance().SetResource(new_texture);
      cache[cache_idx] = new_texture->GetID();
    }
    return cache[cache_idx];
  }

  uint64_t GenTextureCube(unsigned int width, unsigned int height, bool with_mipmap, int NR)
  {
    std::shared_ptr<Resource> new_texture(new ResourceTextureCube(width, height, with_mipmap, NR));
    ResourceMgr::GetInstance().SetResource(new_texture);
    return new_texture->GetID();
  }

  uint64_t GenModel(const char* path)
  {
    static std::unordered_map<std::string, uint64_t> cache;
    std::string cache_idx = path;
    if (!cache.count(cache_idx)) {
      std::shared_ptr<Resource> new_model(new ResourceModel(path));
      ResourceMgr::GetInstance().SetResource(new_model);
      cache[cache_idx] = new_model->GetID();
    }
    return cache[cache_idx];
  }

  std::shared_ptr<Resource> ResourceMgr::GetResource(uint64_t id)
  {
    if (_res.count(id)) {
      return _res[id];
    }

    return nullptr;
  }

  void ResourceMgr::SetResource(std::shared_ptr<Resource> res)
  {
    auto new_id = GenResourceId();
    res->SetID(new_id);

    _res[new_id] = res;
  }

  void ResourceMgr::Load()
  {
    for (auto& res : _res) {
      if (!res.second->IsLoaded()) {
        res.second->Load();
      }
    }
  }

  void ResourceMgr::Load(uint64_t id)
  {
    _res[id]->Load();
  }

}