#pragma once

#include <cstdint>
#include <unordered_map>
#include <memory>

namespace render {

  uint64_t GenTexture2D(unsigned int width, unsigned int height, bool with_mipmap = false, int NR = 3);
  uint64_t GenTexture2DFromFile(const char* path, bool with_mipmap = false, bool is_hdr = false);
  uint64_t GenTextureCube(unsigned int width, unsigned int height, bool with_mipmap = false, int NR = 3);
  uint64_t GenModel(const char* path);

  class Resource;

  class ResourceMgr {
  public:
    static ResourceMgr& GetInstance() {
      static ResourceMgr inst;
      return inst;
    }

    std::shared_ptr<Resource> GetResource(uint64_t id);
    void SetResource(std::shared_ptr<Resource> res);

    template<typename T>
    std::shared_ptr<T> GetResourceAs(uint64_t id) {
      auto res = GetResource(id);
      return std::dynamic_pointer_cast<T>(res);
    }

    void Load();
    void Load(uint64_t id);

  private:
    ResourceMgr() {}

    uint64_t GenResourceId() {
      static uint64_t base_id = 0;
      return ++base_id;
    }

    std::unordered_map<uint64_t, std::shared_ptr<Resource>> _res;
  };

}