#include <memory>
#include <vector>
#include <random>
#include <chrono>

#include "render.h"

#include "Shader.h"
#include "Model.h"
#include "resource.h"
#include "resource_mgr.h"
#include "resource_utils.h"
#include "utils.h"
#include "imgui.h"

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

  std::vector<glm::mat4> getPointLightVP(const glm::vec3& pos) {
    std::vector<glm::mat4> res;
    auto projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    return res;
  }

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
    auto begin_time = std::chrono::steady_clock::now();
    if (_enable_shadow) {
      RenderShadow();
    }
    auto end_shadow = std::chrono::steady_clock::now();
    RenderGbuffer();
    auto end_gbuffer = std::chrono::steady_clock::now();
    if (_enable_ssao) {
      RenderSSAO();
    }
    auto end_ssao = std::chrono::steady_clock::now();
    RenderLight();
    auto end_light = std::chrono::steady_clock::now();
    RenderSkyBox();
    auto end_skybox = std::chrono::steady_clock::now();

    _dt_shadow_pass = std::chrono::duration<float, std::milli>(end_shadow - begin_time).count();
    _dt_gbuffer_pass = std::chrono::duration<float, std::milli>(end_gbuffer - end_shadow).count();
    _dt_ssao_pass = std::chrono::duration<float, std::milli>(end_ssao - end_gbuffer).count();
    _dt_light_pass = std::chrono::duration<float, std::milli>(end_light - end_ssao).count();
    _dt_skybox_pass = std::chrono::duration<float, std::milli>(end_skybox - end_light).count();

    ImGui::Text("shadow pass: %.3f ms", _dt_shadow_pass);
    ImGui::Text("gbuffer pass: %.3f ms", _dt_gbuffer_pass);
    ImGui::Text("ssao pass: %.3f ms", _dt_ssao_pass);
    ImGui::Text("light pass: %.3f ms", _dt_light_pass);
    ImGui::Text("skybox pass: %.3f ms", _dt_skybox_pass);

    ImGui::Checkbox("Enable Shadow", &_enable_shadow);
    ImGui::Checkbox("Enable SSAO", &_enable_ssao);
  }

  void Render::Init()
  {
    InitShader();
    InitObjects();
    InitPBR();
    InitShadowMap();
    InitSSAO();
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
    _point_light[new_light.light_id].shadow_map_idx = -1;
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
    _direction_light[new_light.light_id].shadow_map_idx = -1;
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
    _max_direction_light_shadow = 3;
    _max_point_light_shadow = 3;
    _shadow_map_width = 2048;
    _shadow_map_height = 2048;

    _enable_shadow = true;
    _enable_ssao = true;
  }
  void Render::RenderShadow()
  {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glBindFramebuffer(GL_FRAMEBUFFER, _shadow_frame_buffer);
    glViewport(0, 0, _shadow_map_width, _shadow_map_height);

    // point_light
    _shadow_shader_point->Use();
    _point_shadow_count = 0;
    for (auto& light : _point_light) {
      if (light.second.enable_shadow) {
        // each light
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _point_shadow_map[_point_shadow_count], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glClear(GL_DEPTH_BUFFER_BIT);

        _shadow_shader_point->SetFV3("lightPos", glm::value_ptr(light.second.position));
        _shadow_shader_point->SetFloat("far_plane", 100.0f);
        auto vps = getPointLightVP(light.second.position);
        light.second.vps = vps;
        light.second.shadow_map_idx = _point_shadow_count;
        for (int i = 0; i < vps.size(); i++) {
          // each face
          std::string uniform_name = "shadowMatrices[" + std::to_string(i) + "]";
          _shadow_shader_point->SetFM4(uniform_name.c_str(), glm::value_ptr(vps[i]));
        }

        for (auto& obj : _render_objects) {
          _shadow_shader_point->SetFM4("model", glm::value_ptr(obj.second.transform));

          auto mesh = GetModelResource(obj.second.mesh);
          mesh->Draw(_shadow_shader_point);
        }

        _point_shadow_count++;
      }
    }

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    // direction_light
    _shadow_shader_direction->Use();
    _diretion_shadow_count = 0;
    for (auto& light : _direction_light) {
      if (light.second.enable_shadow) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _diretion_shadow_map[_diretion_shadow_count], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glClear(GL_DEPTH_BUFFER_BIT);

        auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), light.second.direction, glm::vec3(0.0f, 1.0f, 0.0f));
        auto projection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, -20.0f, 20.0f);
        light.second.vp = projection * view;
        light.second.shadow_map_idx = _diretion_shadow_count;
        auto vp = projection * view;
        _shadow_shader_direction->SetFM4("shadow_vp", glm::value_ptr(light.second.vp));
        for (auto& obj : _render_objects) {
          _shadow_shader_direction->SetFM4("model", glm::value_ptr(obj.second.transform));

          auto mesh = GetModelResource(obj.second.mesh);
          mesh->Draw(_shadow_shader_point);
        }

        _diretion_shadow_count++;
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
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
  void Render::RenderSSAO()
  {
    _ssao->Use();
    glBindFramebuffer(GL_FRAMEBUFFER, _ssao_frame_buffer);
    glViewport(0, 0, _windows_width, _windows_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // gbuffer
    auto view_postion_texture = GetTexture2DResource(_g_view_position);
    auto view_normal_texture = GetTexture2DResource(_g_view_normal);
    view_postion_texture->BindToTexture(1);
    view_normal_texture->BindToTexture(2);
    _ssao->SetInt("gViewPos", 1);
    _ssao->SetInt("gViewNor", 2);

    // noise
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _ssao_noise_map);
    _ssao->SetInt("texture_noise", 3);


    // kernel
    for (int i = 0; i < _ssao_kernal.size(); i++) {
      std::string idx_name = "samples[" + std::to_string(i) + "]";
      _ssao->SetFV3(idx_name.c_str(), glm::value_ptr(_ssao_kernal[i]));
    }

    // view, projection
    _ssao->SetFM4("projection", glm::value_ptr(_camera_projection));

    renderQuad();
  }
  void Render::RenderLight()
  {
    _light->Use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _windows_width, _windows_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _light->SetFV3("cam_pos", glm::value_ptr(_camera_pos));

    _light->SetInt("enable_ssao", _enable_ssao ? 1 : 0);
    _light->SetInt("enable_shadow", _enable_shadow ? 1 : 0);

    _light->SetInt("point_light_count", _point_light.size());
    int idx = 0;
    for (const auto& p_light : _point_light) {
      std::string base_name = "point_light_list[" + std::to_string(idx) + "]";
      _light->SetFV3((base_name + ".position").c_str(), glm::value_ptr(p_light.second.position));
      _light->SetFV3((base_name + ".diffuse").c_str(), glm::value_ptr(p_light.second.color));
      _light->SetInt((base_name + ".shadow_map_idx").c_str(), p_light.second.shadow_map_idx);
      idx++;
    }

    int point_shadow_delta_base = 10;
    for (int i = 0; i < _point_shadow_count; i++) {
      glActiveTexture(GL_TEXTURE0 + point_shadow_delta_base + i);
      glBindTexture(GL_TEXTURE_CUBE_MAP, _point_shadow_map[i]);
    }

    _light->SetInt("direction_light_count", _direction_light.size());
    idx = 0;
    for (const auto& d_light : _direction_light) {
      std::string base_name = "direction_light_list[" + std::to_string(idx) + "]";
      _light->SetFV3((base_name + ".direction").c_str(), glm::value_ptr(d_light.second.direction));
      _light->SetFV3((base_name + ".diffuse").c_str(), glm::value_ptr(d_light.second.color));
      _light->SetFM4((base_name + ".shadow_vp").c_str(), glm::value_ptr(d_light.second.vp));
      _light->SetInt((base_name + ".shadow_map_idx").c_str(), d_light.second.shadow_map_idx);
      idx++;
    }

    int direction_shadow_delta_base = 15;
    for (int i = 0; i < _diretion_shadow_count; i++) {
      glActiveTexture(GL_TEXTURE0 + direction_shadow_delta_base + i);
      glBindTexture(GL_TEXTURE_2D, _diretion_shadow_map[i]);
    }

    auto position_ao_texture = GetTexture2DResource(_g_position_ao);
    auto albedo_roughness_texture = GetTexture2DResource(_g_albedo_roughness);
    auto normal_metalic_texture = GetTexture2DResource(_g_normal_metalic);
    auto irradiance_texture = GetTextureCubeResource(_pbr_texture_irradiance);
    auto prefilter_texture = GetTextureCubeResource(_pbr_texture_prefilter);
    auto brdf_texture = GetTexture2DResource(_pbr_texture_brdf);

    position_ao_texture->BindToTexture(1);
    albedo_roughness_texture->BindToTexture(2);
    normal_metalic_texture->BindToTexture(3);
    irradiance_texture->BindToTexture(4);
    prefilter_texture->BindToTexture(5);
    brdf_texture->BindToTexture(6);

    _light->SetInt("gPosAO", 1);
    _light->SetInt("gAlbedoRoughness", 2);
    _light->SetInt("gNormalMetalic", 3);
    _light->SetInt("irradiance_map", 4);
    _light->SetInt("prefilter_map", 5);
    _light->SetInt("brdf_lut", 6);

    // SSAO
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, _ssao_map);
    _light->SetInt("gSSAO", 7);

    for (int i = 0; i < 5; i++) {
      std::string base_name = "point_light_shadow[" + std::to_string(i) + "]";
      _light->SetInt(base_name.c_str(), point_shadow_delta_base + i);
    }

    for (int i = 0; i < 5; i++) {
      std::string base_name = "direction_light_shadow[" + std::to_string(i) + "]";
      _light->SetInt(base_name.c_str(), direction_shadow_delta_base + i);
    }

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
    auto view_position_texture = GetTexture2DResource(_g_view_position);
    auto view_normal_texture = GetTexture2DResource(_g_view_normal);

    glBindFramebuffer(GL_FRAMEBUFFER, _gbuffer_frame_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      position_ao_texture->GetTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
      albedo_roughness_texture->GetTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
      normal_metalic_texture->GetTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
      view_position_texture->GetTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D,
      view_normal_texture->GetTexture(), 0);
    unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(5, attachments);
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
    delete _shadow_shader_point;
    delete _shadow_shader_direction;

    _pbr_hdr_preprocess = new Shader("shader/cube_sampler_vs.glsl", "shader/pbr_hdr_preprocess_fs.glsl");
    _pbr_irradiance = new Shader("shader/cube_sampler_vs.glsl", "shader/pbr_irradiance_fs.glsl");
    _pbr_prefilter = new Shader("shader/cube_sampler_vs.glsl", "shader/pbr_prefilter_fs.glsl");
    _pbr_brdf = new Shader("shader/quad_sampler_vs.glsl", "shader/pbr_brdf_fs.glsl");
    _gbuffer = new Shader("shader/gbuffer_vs.glsl", "shader/gbuffer_fs.glsl");
    _light = new Shader("shader/quad_sampler_vs.glsl", "shader/pbr_fs.glsl");
    _skybox = new Shader("shader/skybox.vert", "shader/skybox.frag");
    _shadow_shader_point = new Shader("shader/shadow_point_vs.glsl", "shader/shadow_point_gs.glsl", "shader/shadow_point_fg.glsl");
    _shadow_shader_direction = new Shader("shader/shadow_vs.glsl", "shader/shadow_fg.glsl");
    _ssao = new Shader("shader/quad_sampler_vs.glsl", "shader/ssao_fs.glsl");
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
    _g_view_position = GenTexture2D(_windows_width, _windows_height);
    _g_view_normal = GenTexture2D(_windows_width, _windows_height);
  }
  void Render::InitPBR()
  {
    InitPbrRenderBuffer();
  }
  void Render::InitShadowMap()
  {
    _point_shadow_count = 0;
    _diretion_shadow_count = 0;

    for (int i = _point_shadow_map.size(); i < _max_point_light_shadow; i++) {
      _point_shadow_map.push_back(GenShadowMap(2));
    }

    for (int i = _diretion_shadow_map.size(); i < _max_direction_light_shadow; i++) {
      _diretion_shadow_map.push_back(GenShadowMap(1));
    }

    glGenFramebuffers(1, &_shadow_frame_buffer);
  }
  std::vector<glm::vec3> GenSSAONoise(int width, int height) {
    std::vector<glm::vec3> res;

    std::uniform_real_distribution random_floats(0.0f, 1.0f);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoNoise;

    for (int i = 0; i < width * height; i++) {
      res.emplace_back(
        random_floats(generator) * 2.0f - 1.0f, 
        random_floats(generator) * 2.0f - 1.0f, 
        0.0f);
    }

    return res;
  }
  std::vector<glm::vec3> GenSSAOKernel(int width, int height) {
    std::vector<glm::vec3> res;

    std::uniform_real_distribution random_floats(0.0f, 1.0f);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoNoise;

    auto lerp = [](float a, float b, float ratio) {
      return a + ratio * (b - a);
    };

    int sampler_count = width * height;
    for (int i = 0; i < sampler_count; i++) {
      glm::vec3 sampler(
        random_floats(generator) * 2.0f - 1.0f,
        random_floats(generator) * 2.0f - 1.0f,
        random_floats(generator)
      );

      sampler = glm::normalize(sampler);
      sampler *= random_floats(generator);

      float scale = (float)i / sampler_count;
      sampler *= lerp(0.1f, 1.0f, scale * scale);

      res.push_back(sampler);
    }

    return res;
  }
  void Render::InitSSAO()
  {
    _ssao_map = render::genTexture2D(_windows_width, _windows_height, false, 1);

    glGenFramebuffers(1, &_ssao_frame_buffer);
    glGenRenderbuffers(1, &_ssao_render_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _ssao_frame_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _ssao_render_buffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _windows_width, _windows_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _ssao_render_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ssao_map, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // noise
    auto noise_list = GenSSAONoise(4, 4);
    glGenTextures(1, &_ssao_noise_map);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _ssao_noise_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise_list.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // kernel
    _ssao_kernal = GenSSAOKernel(16, 16);
  }
  unsigned int Render::GenShadowMap(int light_type)
  {
    unsigned int res;
    glGenTextures(1, &res);

    if (light_type == 1) {
      // direction light
      glBindTexture(GL_TEXTURE_2D, res);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _shadow_map_width, _shadow_map_height,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glBindTexture(GL_TEXTURE_2D, 0);
    } else if (light_type == 2) {
      // point light
      glBindTexture(GL_TEXTURE_CUBE_MAP, res);
      for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
          _shadow_map_width, _shadow_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      }
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    return res;
  }
}