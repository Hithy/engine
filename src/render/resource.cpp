#include "resource.h"

#include <unordered_map>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "utils.h"
#include "Model.h"

#include "stb_image.h"

namespace render {

  ResourceTexture2D::ResourceTexture2D(const char* path, bool with_mipmap, bool is_hdr)
    : Resource(ResourceType::Texture2D)
    , _loaded(false)
    , _path(path)
    , _width(0)
    , _height(0)
    , _with_mipmap(with_mipmap)
    , _channel_count(0)
    , _gl_texture(0)
    , _flip_vectical(false)
    , _is_hdr(is_hdr)
  {
  }

  ResourceTexture2D::ResourceTexture2D(int width, int height, bool with_mipmap, int NR, bool is_hdr)
    : Resource(ResourceType::Texture2D)
    , _loaded(false)
    , _path("")
    , _width(width)
    , _height(height)
    , _with_mipmap(with_mipmap)
    , _channel_count(NR)
    , _gl_texture(0)
    , _flip_vectical(false)
    , _is_hdr(is_hdr)
  {
  }

  void ResourceTexture2D::Load()
  {
    if (IsLoaded()) {
      return;
    }

    if (_path.size()) {
      LoadFromFile();
    }
    else {
      LoadNewTexture();
    }

    SetLoaded();
  }

  bool ResourceTexture2D::BindToTexture(unsigned int texture_idx)
  {
    if (!IsLoaded()) {
      return false;
    }

    glActiveTexture(GL_TEXTURE0 + texture_idx);
    glBindTexture(GL_TEXTURE_2D, _gl_texture);

    return true;
  }

  bool ResourceTexture2D::BindToCurrentTexture()
  {
    if (!IsLoaded()) {
      return false;
    }

    glBindTexture(GL_TEXTURE_2D, _gl_texture);

    return true;
  }

  void ResourceTexture2D::LoadFromFile()
  {
    int _channel_count;
    void* data;

    stbi_set_flip_vertically_on_load(_flip_vectical);
    if (_is_hdr) {
      data = stbi_loadf(_path.c_str(), &_width, &_height, &_channel_count, 0);
    }
    else {
      data = stbi_load(_path.c_str(), &_width, &_height, &_channel_count, 0);
    }
    if (data) {
      _gl_texture = render::genTexture2D(_width, _height, _with_mipmap, _channel_count, data, _is_hdr);
      stbi_image_free(data);
    }
  }

  void ResourceTexture2D::LoadNewTexture()
  {
    _gl_texture = render::genTexture2D(_width, _height, _with_mipmap, _channel_count);
  }

  ResourceTextureCube::ResourceTextureCube(int width, int height, bool with_mipmap, int NR)
    : Resource(ResourceType::Texture2D)
    , _loaded(false)
    , _width(width)
    , _height(height)
    , _with_mipmap(with_mipmap)
    , _channel_count(NR)
    , _gl_texture(0)
  {
  }

  void ResourceTextureCube::Load()
  {
    if (IsLoaded()) {
      return;
    }

    _gl_texture = render::genTextureCube(_width, _height, _with_mipmap, _channel_count);

    SetLoaded();
  }

  bool ResourceTextureCube::BindToTexture(unsigned int texture_idx)
  {
    if (!IsLoaded()) {
      return false;
    }

    glActiveTexture(GL_TEXTURE0 + texture_idx);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _gl_texture);

    return true;
  }

  bool ResourceTextureCube::BindToCurrentTexture()
  {
    if (!IsLoaded()) {
      return false;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, _gl_texture);

    return true;
  }

  void ResourceTextureCube::GenMipmap()
  {
    if (!IsLoaded() || !_with_mipmap) {
      return;
    }
    BindToTexture(0);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  }

  ResourceModel::ResourceModel(const char* path)
    : Resource(ResourceType::Model)
    , _path(path)
    , _loaded(false)
    , _model_ptr(nullptr)
    , _diffuse_path("")
    , _normal_path("")
  {
  }

  void ResourceModel::Load()
  {
    if (IsLoaded()) {
      return;
    }
    _model_ptr = new Model(_path.c_str());
    SetLoaded();
  }

  void ResourceModel::Draw(Shader* shader)
  {
    if (!IsLoaded()) {
      return;
    }

    _model_ptr->Draw(shader);
  }

}