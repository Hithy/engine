#pragma once

#include <cstdint>
#include <string>

namespace render {
  class Model;
  class Shader;

  enum class ResourceType {
    None = 0,
    Model,
    // Mesh,
    Texture2D,
    TextureCube,
  };

  class Resource {
  public:
    Resource() : _id(0), _type(static_cast<int>(ResourceType::None)) {}
    Resource(ResourceType type) : _id(0), _type(static_cast<int>(type)) {}
    Resource(ResourceType type, uint64_t id) : _id(id), _type(static_cast<int>(type)) {}

    // setter & getter
    uint64_t GetID() { return _id; }
    void SetID(uint64_t id) { _id = id; }
    int GetType() { return _type; }
    void SetType(ResourceType type) { _type = static_cast<int>(type); }

    // interface
    virtual void Load() = 0;
    virtual bool IsLoaded() = 0;

  private:
    uint64_t _id;
    int _type;
  };

  class ResourceTexture2D : public Resource {
  public:
    ResourceTexture2D(const char* path, bool with_mipmap = true, bool is_hdr = false);
    ResourceTexture2D(int width, int height, bool with_mipmap, int NR, bool is_hdr = false);

    // interface
    void Load() override;
    bool IsLoaded() override { return _loaded; }

    bool BindToTexture(unsigned int texture_idx);
    bool BindToCurrentTexture();

    void SetFlipVectical(bool enable) { _flip_vectical = enable; }

    unsigned int GetTexture() { return _gl_texture; }

  private:
    void SetLoaded() { _loaded = true; }

    void LoadFromFile();
    void LoadNewTexture();

  private:
    bool _loaded;

    // load texture from file
    std::string _path;

    // gen new texture
    int _width;
    int _height;
    bool _with_mipmap;
    int _channel_count;

    bool _flip_vectical;
    bool _is_hdr;

    unsigned int _gl_texture;
  };

  class ResourceTextureCube : public Resource {
  public:
    ResourceTextureCube(int width, int height, bool with_mipmap, int NR);

    // interface
    void Load() override;
    bool IsLoaded() override { return _loaded; }

    bool BindToTexture(unsigned int texture_idx);
    bool BindToCurrentTexture();

    unsigned int GetTexture() { return _gl_texture; }

  private:
    void SetLoaded() { _loaded = true; }

  private:
    bool _loaded;

    int _width;
    int _height;
    bool _with_mipmap;
    int _channel_count;

    unsigned int _gl_texture;
  };

  
  class ResourceModel : public Resource {
  public:
    ResourceModel(const char* path);

    // interface
    void Load() override;
    bool IsLoaded() override { return _loaded; }

    void Draw(Shader* shader);

  private:
    void SetLoaded() { _loaded = true; }

  private:
    bool _loaded;

    std::string _path;

    std::string _diffuse_path;
    std::string _normal_path;

    Model* _model_ptr;
  };
}