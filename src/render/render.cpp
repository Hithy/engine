#include <memory>

#include "render.h"

#include "Shader.h"
#include "Model.h"
#include "resource.h"
#include "resource_mgr.h"
#include "resource_utils.h"
#include "utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glad/glad.h>

namespace render {
  static glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  static glm::mat4 captureViews[] =
  {
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
  };

  void Render::PrepareRender()
  {
    // 1. skybox
    InitPbrSkybox();

    // 2. irradiance
    InitPbrIrradiance();

    // 3. prefilter
    InitPbrPrefilter();

    // 4. brdf
    InitPbrBrdf();
  }

  void Render::DoRender()
  {
    // InitPbrIrradiance();
    RenderGbuffer();
    RenderLight();
    RenderSkyBox();
  }

  void Render::Init()
  {
    InitShader();
    InitObjects();
    InitPBR();
  }

  void Render::SetPbrSkyBox(const char* path)
  {
    _pbr_skybox_path = path;
  }

  void Render::SetCameraTrans(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& pos)
  {
    _camera_view = view;
    _camera_projection = projection;
    _camera_pos = pos;
  }

  void Render::AddRenderItem(const RenderItem& new_item)
  {
    assert(_render_objects.count(new_item.obj_id) == 0);
    _render_objects[new_item.obj_id] = new_item;
  }

  void Render::DelRenderItem(uint64_t obj_id)
  {
    auto itr = _render_objects.find(obj_id);
    if (itr != _render_objects.end()) {
      _render_objects.erase(itr);
    }
  }

  void Render::ClearRenderItem()
  {
    _render_objects.clear();
  }

  void Render::AddPointLight(const RenderPointLight& new_light)
  {
    _point_light[new_light.light_id] = new_light;
  }

  void Render::DelPointLight(uint64_t light_id)
  {
    _point_light.erase(light_id);
  }

  void Render::ClearPointLight()
  {
    _point_light.clear();
  }

  void Render::AddDirectionLight(const RenderDirectionLight& new_light)
  {
    _direction_light[new_light.light_id] = new_light;
  }

  void Render::DelDirectionLight(uint64_t light_id)
  {
    _direction_light.erase(light_id);
  }

  void Render::ClearDirectionLight()
  {
    _direction_light.clear();
  }

  Render::Render()
  {
    // shader
    _pbr_hdr_preprocess = nullptr;
    _pbr_irradiance = nullptr;
    _pbr_prefilter = nullptr;
    _pbr_brdf = nullptr;
    _gbuffer = nullptr;
    _light = nullptr;
    _skybox = nullptr;

    // config
    _pbr_skybox_width = 512;
    _pbr_skybox_height = 512;
    _pbr_irradiance_width = 32;
    _pbr_irradiance_height = 32;
    _pbr_prefilter_width = 128;
    _pbr_prefilter_height = 128;
    _pbr_brdf_width = 512;
    _pbr_brdf_height = 512;
    _windows_width = 1920;
    _windows_height = 1080;
  }
  void Render::RenderGbuffer()
  {
    // input: mvp, framebuffer, map
    _gbuffer->Use();
    _gbuffer->SetFM4("view", glm::value_ptr(_camera_view));
    _gbuffer->SetFM4("projection", glm::value_ptr(_camera_projection));

    glBindFramebuffer(GL_FRAMEBUFFER, _gbuffer_frame_buffer);
    glViewport(0, 0, _windows_width, _windows_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& obj : _render_objects) {
      _gbuffer->SetFM4("model", glm::value_ptr(obj.second.transform));

      auto albedo_map = GetTexture2DResource(obj.second.albedo);
      auto normal_map = GetTexture2DResource(obj.second.normal);
      auto metalic_map = GetTexture2DResource(obj.second.metalic);
      auto roughness_map = GetTexture2DResource(obj.second.roughness);
      auto ao_map = GetTexture2DResource(obj.second.ao);
      auto mesh = GetModelResource(obj.second.mesh);

      albedo_map->BindToTexture(0);
      normal_map->BindToTexture(1);
      metalic_map->BindToTexture(2);
      roughness_map->BindToTexture(3);
      ao_map->BindToTexture(4);

      _gbuffer->SetInt("albedo", 0);
      _gbuffer->SetInt("normal", 1);
      _gbuffer->SetInt("metalic", 2);
      _gbuffer->SetInt("roughness", 3);
      _gbuffer->SetInt("ao", 4);

      mesh->Draw(_gbuffer);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::RenderLight()
  {
    _light->Use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _windows_width, _windows_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _light->SetFV3("cam_pos", glm::value_ptr(_camera_pos));

    _light->SetInt("point_light_count", _point_light.size());
    int idx = 0;
    for (const auto& p_light : _point_light) {
      std::string base_name = "point_light_list[" + std::to_string(idx) + "]";
      _light->SetFV3((base_name + ".position").c_str(), glm::value_ptr(p_light.second.position));
      _light->SetFV3((base_name + ".diffuse").c_str(), glm::value_ptr(p_light.second.color));
      idx++;
    }

    _light->SetInt("direction_light_count", _direction_light.size());
    idx = 0;
    for (const auto& d_light : _direction_light) {
      std::string base_name = "direction_light_list[" + std::to_string(idx) + "]";
      _light->SetFV3((base_name + ".direction").c_str(), glm::value_ptr(d_light.second.direction));
      _light->SetFV3((base_name + ".diffuse").c_str(), glm::value_ptr(d_light.second.color));
      idx++;
    }

    auto position_ao_texture = GetTexture2DResource(_g_position_ao);
    auto albedo_roughness_texture = GetTexture2DResource(_g_albedo_roughness);
    auto normal_metalic_texture = GetTexture2DResource(_g_normal_metalic);
    auto irradiance_texture = GetTextureCubeResource(_pbr_texture_irradiance);
    auto prefilter_texture = GetTextureCubeResource(_pbr_texture_prefilter);
    auto brdf_texture = GetTexture2DResource(_pbr_texture_brdf);

    position_ao_texture->BindToTexture(0);
    albedo_roughness_texture->BindToTexture(1);
    normal_metalic_texture->BindToTexture(2);
    irradiance_texture->BindToTexture(3);
    prefilter_texture->BindToTexture(4);
    brdf_texture->BindToTexture(5);

    _light->SetInt("gPosAO", 0);
    _light->SetInt("gAlbedoRoughness", 1);
    _light->SetInt("gNormalMetalic", 2);
    _light->SetInt("irradiance_map", 3);
    _light->SetInt("prefilter_map", 4);
    _light->SetInt("brdf_lut", 5);

    renderQuad();
  }
  void Render::RenderSkyBox()
  {
    _skybox->Use();
    _skybox->SetFM4("view", glm::value_ptr(_camera_view));
    _skybox->SetFM4("projection", glm::value_ptr(_camera_projection));
    _skybox->SetInt("skybox", 0);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _gbuffer_frame_buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, _windows_width, _windows_height, 0, 0, _windows_width,
      _windows_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto skybox_texture = GetTextureCubeResource(_pbr_texture_skybox);
    skybox_texture->BindToTexture(0);

    renderBox();
  }
  void Render::InitPbrRenderBuffer()
  {
    glGenRenderbuffers(1, &_pbr_render_buffer);
    glGenFramebuffers(1, &_pbr_frame_buffer);
    glGenRenderbuffers(1, &_gbuffer_render_buffer);
    glGenFramebuffers(1, &_gbuffer_frame_buffer);

    glBindRenderbuffer(GL_RENDERBUFFER, _pbr_render_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _pbr_frame_buffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _pbr_skybox_width, _pbr_skybox_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _pbr_render_buffer);

    glBindRenderbuffer(GL_RENDERBUFFER, _gbuffer_render_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _gbuffer_frame_buffer);

    auto position_ao_texture = GetTexture2DResource(_g_position_ao);
    auto albedo_roughness_texture = GetTexture2DResource(_g_albedo_roughness);
    auto normal_metalic_texture = GetTexture2DResource(_g_normal_metalic);

    glBindFramebuffer(GL_FRAMEBUFFER, _gbuffer_frame_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      position_ao_texture->GetTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
      albedo_roughness_texture->GetTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
      normal_metalic_texture->GetTexture(), 0);
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    glBindRenderbuffer(GL_RENDERBUFFER, _gbuffer_render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _windows_width, _windows_width);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _gbuffer_render_buffer);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::InitPbrSkybox()
  {
    uint64_t hdr_skybox = GenTexture2DFromFile(_pbr_skybox_path.c_str(), false, true);
    auto hdr_skybox_texture = GetTexture2DResource(hdr_skybox, false);
    hdr_skybox_texture->SetFlipVectical(true);
    hdr_skybox_texture->Load();
    auto skybox_texture = GetTextureCubeResource(_pbr_texture_skybox);

    glBindFramebuffer(GL_FRAMEBUFFER, _pbr_frame_buffer);
    glViewport(0, 0, _pbr_skybox_width, _pbr_skybox_height);
    
    _pbr_hdr_preprocess->Use();
    _pbr_hdr_preprocess->SetFM4("projection", glm::value_ptr(captureProjection));
    hdr_skybox_texture->BindToTexture(0);
    _pbr_hdr_preprocess->SetInt("hdrTexture", 0);
    for (int i = 0; i < 6; i++) {
      _pbr_hdr_preprocess->SetFM4("view", glm::value_ptr(captureViews[i]));
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skybox_texture->GetTexture(), 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderBox();
    }
    skybox_texture->GenMipmap();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::InitPbrIrradiance()
  {
    // input
    auto skybox_texture = GetTextureCubeResource(_pbr_texture_skybox);
    // output
    auto irradiance_texture = GetTextureCubeResource(_pbr_texture_irradiance);

    glBindFramebuffer(GL_FRAMEBUFFER, _pbr_frame_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _pbr_render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _pbr_irradiance_width, _pbr_irradiance_height);
    glViewport(0, 0, _pbr_irradiance_width, _pbr_irradiance_height);

    _pbr_irradiance->Use();
    _pbr_irradiance->SetFM4("projection", glm::value_ptr(captureProjection));
    skybox_texture->BindToTexture(0);
    _pbr_irradiance->SetInt("skybox", 0);
    for (int i = 0; i < 6; i++) {
      _pbr_irradiance->SetFM4("view", glm::value_ptr(captureViews[i]));
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_texture->GetTexture(), 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderBox();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::InitPbrPrefilter()
  {
    // input
    auto skybox_texture = GetTextureCubeResource(_pbr_texture_skybox);
    // output
    auto prefilter_texture = GetTextureCubeResource(_pbr_texture_prefilter);

    _pbr_prefilter->Use();
    _pbr_prefilter->SetFM4("projection", glm::value_ptr(captureProjection));
    skybox_texture->BindToTexture(0);
    _pbr_prefilter->SetInt("skybox", 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _pbr_frame_buffer);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
      // reisze framebuffer according to mip-level size.
      unsigned int mipWidth = _pbr_prefilter_width * std::pow(0.5, mip);
      unsigned int mipHeight = _pbr_prefilter_height * std::pow(0.5, mip);
      glBindRenderbuffer(GL_RENDERBUFFER, _pbr_render_buffer);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
      glViewport(0, 0, mipWidth, mipHeight);

      float roughness = (float)mip / (float)(maxMipLevels - 1);
      _pbr_prefilter->SetFloat("roughness", roughness);
      for (unsigned int i = 0; i < 6; ++i)
      {
        _pbr_prefilter->SetFM4("view", glm::value_ptr(captureViews[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
          GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_texture->GetTexture(), mip);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderBox();
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::InitPbrBrdf()
  {
    // input
    auto skybox_texture = GetTextureCubeResource(_pbr_texture_skybox);
    // output
    auto brdf_texture = GetTexture2DResource(_pbr_texture_brdf);

    _pbr_brdf->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, _pbr_frame_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _pbr_render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _pbr_brdf_width, _pbr_brdf_height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_texture->GetTexture(), 0);
    glViewport(0, 0, _pbr_brdf_width, _pbr_brdf_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::InitShader()
  {
    delete _pbr_hdr_preprocess;
    delete _pbr_irradiance;
    delete _pbr_prefilter;
    delete _pbr_brdf;
    delete _gbuffer;
    delete _light;
    delete _skybox;

    _pbr_hdr_preprocess = new Shader("shader/cube_sampler_vs.glsl", "shader/pbr_hdr_preprocess_fs.glsl");
    _pbr_irradiance = new Shader("shader/cube_sampler_vs.glsl", "shader/pbr_irradiance_fs.glsl");
    _pbr_prefilter = new Shader("shader/cube_sampler_vs.glsl", "shader/pbr_prefilter_fs.glsl");
    _pbr_brdf = new Shader("shader/quad_sampler_vs.glsl", "shader/pbr_brdf_fs.glsl");
    _gbuffer = new Shader("shader/gbuffer_vs.glsl", "shader/gbuffer_fs.glsl");
    _light = new Shader("shader/quad_sampler_vs.glsl", "shader/pbr_fs.glsl");
    _skybox = new Shader("shader/skybox.vert", "shader/skybox.frag");
  }
  void Render::InitObjects()
  {
    _pbr_texture_skybox = GenTextureCube(_pbr_skybox_width, _pbr_skybox_height, true);
    _pbr_texture_irradiance = GenTextureCube(_pbr_irradiance_width, _pbr_irradiance_height);
    _pbr_texture_prefilter = GenTextureCube(_pbr_prefilter_width, _pbr_prefilter_height, true);
    _pbr_texture_brdf = GenTexture2D(_pbr_brdf_width, _pbr_brdf_height, false, 2);

    _g_position_ao = GenTexture2D(_windows_width, _windows_height, false, 4);
    _g_albedo_roughness = GenTexture2D(_windows_width, _windows_height, false, 4);
    _g_normal_metalic = GenTexture2D(_windows_width, _windows_height, false, 4);
  }
  void Render::InitPBR()
  {
    InitPbrRenderBuffer();
  }
}