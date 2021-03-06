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
    auto projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 50.0f);
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    res.push_back(projection * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    return res;
  }

  static double Halton_Seq(int index, int base) {
    double f = 1, r = 0;
    while (index > 0) {
      f = f / base;
      r = r + f * (index % base);
      index = index / base;
    }
    return r;
  }

  static std::vector<glm::vec2> gen_halton23() {
    std::vector<glm::vec2> res;
    for (int i = 1; i <= 16; i++) {
      res.push_back(
        { Halton_Seq(i, 2), Halton_Seq(i, 3) }
      );
    }
    return res;
  }

  glm::vec2 GetHalton(int idx) {
    static auto halton23_list = gen_halton23();
    auto res = halton23_list[idx % 16];
    return glm::vec2(res.x - 0.5, res.y - 0.5) * 1.0f;
  }

  void Render::PrepareRender()
  {
    if (_enable_ibl) {
      // 1. skybox
      InitPbrSkybox();

      // 2. irradiance
      InitPbrIrradiance();

      // 3. prefilter
      InitPbrPrefilter();

      // 4. brdf
      InitPbrBrdf();
    }
  }

  void Render::Update()
  {
  }

  void Render::PostUpdate()
  {
    PostUpdateTAA();
  }

  void Render::DoRender()
  {
    Update();
    auto begin_time = std::chrono::steady_clock::now();
    ComputeClusterLight();
    auto end_cluster_box = std::chrono::steady_clock::now();
    RenderShadow();
    auto end_shadow = std::chrono::steady_clock::now();
    RenderGbuffer();
    auto end_gbuffer = std::chrono::steady_clock::now();
    RenderSSAO();
    auto end_ssao = std::chrono::steady_clock::now();
    RenderLight();
    auto end_light = std::chrono::steady_clock::now();
    RenderSkyBox();
    auto end_skybox = std::chrono::steady_clock::now();
    RenderTAA();
    RenderPost();
    PostUpdate();

    _dt_cluster_box_pass = std::chrono::duration<float, std::milli>(end_cluster_box - begin_time).count();
    _dt_shadow_pass = std::chrono::duration<float, std::milli>(end_shadow - end_cluster_box).count();
    _dt_gbuffer_pass = std::chrono::duration<float, std::milli>(end_gbuffer - end_shadow).count();
    _dt_ssao_pass = std::chrono::duration<float, std::milli>(end_ssao - end_gbuffer).count();
    _dt_light_pass = std::chrono::duration<float, std::milli>(end_light - end_ssao).count();
    _dt_skybox_pass = std::chrono::duration<float, std::milli>(end_skybox - end_light).count();

    ImGui::Text("cluster pass: %.3f ms", _dt_cluster_box_pass);
    ImGui::Text("shadow pass: %.3f ms", _dt_shadow_pass);
    ImGui::Text("gbuffer pass: %.3f ms", _dt_gbuffer_pass);
    ImGui::Text("ssao pass: %.3f ms", _dt_ssao_pass);
    ImGui::Text("light pass: %.3f ms", _dt_light_pass);
    ImGui::Text("skybox pass: %.3f ms", _dt_skybox_pass);

    ImGui::Checkbox("Enable Shadow", &_enable_shadow);
    ImGui::Checkbox("Enable SSAO", &_enable_ssao);

    ImGui::SliderFloat("TAA Blend Ratio", &_taa_blend_ratio, 0.0f, 1.0f);
    ImGui::SliderFloat("TAA Jitter Ratio", &_taa_jitter_ratio, 0.0f, 1.0f);
  }

  void Render::Init()
  {
    InitShader();
    InitObjects();
    InitPBR();
    InitShadowMap();
    InitSSAO();
    InitCluster();
    InitTAA();
  }

  void Render::SetPbrSkyBox(const char* path)
  {
    _pbr_skybox_path = path;
    _enable_ibl = _pbr_skybox_path.size() > 0;
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
    _shadow_map_width = 1024;
    _shadow_map_height = 1024;

    _z_near = 0.1f;
    _z_far = 200.0f;
    _z_slices = 20;
    _tile_size = 64;

    _tile_x = (_windows_width + _tile_size - 1) / _tile_size;
    _tile_y = (_windows_height + _tile_size - 1) / _tile_size;

    _taa_jitter_idx = 0;
    _taa_blend_ratio = 0.9f;
    _taa_jitter_ratio = 1.0f;

    _enable_shadow = true;
    _enable_ssao = false;
    _enable_ibl = false;
  }
  void Render::PostUpdateTAA()
  {
    _taa_jitter_idx++;
  }
  void Render::RenderShadow()
  {
    if (!_enable_shadow) {
      return;
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glBindFramebuffer(GL_FRAMEBUFFER, _shadow_frame_buffer);
    glViewport(0, 0, _shadow_map_width, _shadow_map_height);

    // point_light
    _shadow_shader_point->Use();
    _point_shadow_count = 0;
    int cluster_index = 0;
    for (auto& light : _point_light) {
      if (_point_shadow_count >= _max_point_light_shadow)
      {
        break;
      }
      if (light.second.enable_shadow) {
        // each light
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _point_shadow_map[_point_shadow_count], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glClear(GL_DEPTH_BUFFER_BIT);

        _shadow_shader_point->SetFV3("lightPos", glm::value_ptr(light.second.position));
        _shadow_shader_point->SetFloat("far_plane", 50.0f);
        _shadow_shader_point->SetFloat("radius", light.second.radius);
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
        _cluster_point_lights[cluster_index].shadow_idx = _point_shadow_count;
        _point_shadow_count++;
      }
      cluster_index++;
    }

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    // direction_light
    _shadow_shader_direction->Use();
    _diretion_shadow_count = 0;
    for (auto& light : _direction_light) {
      if (_diretion_shadow_count >= _max_direction_light_shadow) {
        break;
      }
      if (light.second.enable_shadow) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _diretion_shadow_map[_diretion_shadow_count], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glClear(GL_DEPTH_BUFFER_BIT);

        auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), light.second.direction, glm::vec3(0.0f, 1.0f, 0.0f));
        auto projection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, -25.0f, 25.0f);
        light.second.vp = projection * view;
        light.second.shadow_map_idx = _diretion_shadow_count;
        _shadow_shader_direction->SetFM4("shadow_vp", glm::value_ptr(light.second.vp));
        for (auto& obj : _render_objects) {
          _shadow_shader_direction->SetFM4("model", glm::value_ptr(obj.second.transform));

          auto mesh = GetModelResource(obj.second.mesh);
          mesh->Draw(_shadow_shader_direction);
        }

        _diretion_shadow_count++;
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::RenderGbuffer()
  {
    static glm::mat4 last_vp = _camera_projection * _camera_view;
    glBindFramebuffer(GL_FRAMEBUFFER, _gbuffer_frame_buffer);
    glViewport(0, 0, _windows_width, _windows_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);

    // input: mvp, framebuffer, map
    _gbuffer->Use();
    _gbuffer->SetFM4("view", glm::value_ptr(_camera_view));
    _gbuffer->SetFM4("projection", glm::value_ptr(_camera_projection));
    auto jitter_base = GetHalton(_taa_jitter_idx) * _taa_jitter_ratio;
    _gbuffer->SetFV2("jitter", glm::value_ptr(glm::vec2(jitter_base.x / _windows_width, jitter_base.y / _windows_height)));

    for (auto& obj : _render_objects) {
      _gbuffer->SetFM4("model", glm::value_ptr(obj.second.transform));
      _gbuffer->SetFM4("last_mvp", glm::value_ptr(last_vp * obj.second.last_trans));

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

    last_vp = _camera_projection * _camera_view;

    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void Render::RenderSSAO()
  {
    if (!_enable_ssao) {
      return;
    }
    _ssao->Use();

    auto jitter_base = GetHalton(_taa_jitter_idx) * _taa_jitter_ratio;
    _gbuffer->SetFV2("jitter", glm::value_ptr(glm::vec2(jitter_base.x / _windows_width, jitter_base.y / _windows_height)));

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
    glBindFramebuffer(GL_FRAMEBUFFER, _taa_jitter_fbo);
    glViewport(0, 0, _windows_width, _windows_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _light->Use();
    _light->SetFV3("cam_pos", glm::value_ptr(_camera_pos));
    _light->SetFM4("cam_view", glm::value_ptr(_camera_view));

    _light->SetInt("enable_ssao", _enable_ssao ? 1 : 0);
    _light->SetInt("enable_shadow", _enable_shadow ? 1 : 0);
    _light->SetInt("enable_ibl", _enable_ibl ? 1 : 0);

    // shadow
    int point_shadow_delta_base = 10;
    int direction_shadow_delta_base = 20;
    
    for (int i = 0; i < _max_point_light_shadow; i++) {
      std::string point_light_map = "point_light_shadow[" + std::to_string(i) + "].shadow_map";
      _light->SetInt(point_light_map.c_str(), point_shadow_delta_base);
    }

    for (int i = 0; i < _max_direction_light_shadow; i++) {
      std::string direction_light_map = "direction_light_shadow[" + std::to_string(i) + "].shadow_map";
      _light->SetInt(direction_light_map.c_str(), direction_shadow_delta_base);
    }

    _light->SetInt("point_light_count", _point_light.size());
    int idx = 0;
    int shadow_idx = 0;
    for (const auto& p_light : _point_light) {
      bool enable_shadow = _enable_shadow && p_light.second.enable_shadow && shadow_idx < _max_point_light_shadow;
      if (enable_shadow) {
        glActiveTexture(GL_TEXTURE0 + point_shadow_delta_base + shadow_idx);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _point_shadow_map[p_light.second.shadow_map_idx]);
        std::string shadow_name = "point_light_shadow[" + std::to_string(shadow_idx) + "]";
        _light->SetInt((shadow_name + ".shadow_map").c_str(), point_shadow_delta_base + shadow_idx);
        shadow_idx++;
      }
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _point_light_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, _cluster_point_lights.size() * sizeof(PLight), _cluster_point_lights.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _light_grid_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _point_light_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _point_light_idx_ssbo);*/
    _light->SetUInt("screen_width", _windows_width);
    _light->SetUInt("screen_height", _windows_height);
    _light->SetUInt("tile_size", _tile_size);
    _light->SetFloat("z_near", _z_near);
    _light->SetFloat("z_far", _z_far);
    _light->SetUInt("z_slices", _z_slices);
    _light->SetUInt("tile_x", _tile_x);
    _light->SetUInt("tile_y", _tile_y);

    _light->SetInt("direction_light_count", _direction_light.size());
    idx = 0;
    shadow_idx = 0;
    for (const auto& d_light : _direction_light) {
      std::string base_name = "direction_light_list[" + std::to_string(idx) + "]";
      bool enable_shadow = _enable_shadow && d_light.second.enable_shadow && shadow_idx < _max_direction_light_shadow;

      _light->SetFV3((base_name + ".direction").c_str(), glm::value_ptr(d_light.second.direction));
      _light->SetFV3((base_name + ".diffuse").c_str(), glm::value_ptr(d_light.second.color));
      _light->SetInt((base_name + ".shadow_idx").c_str(), enable_shadow ? shadow_idx : -1);
      if (enable_shadow) {
        glActiveTexture(GL_TEXTURE0 + direction_shadow_delta_base + shadow_idx);
        glBindTexture(GL_TEXTURE_2D, _diretion_shadow_map[d_light.second.shadow_map_idx]);
        std::string shadow_name = "direction_light_shadow[" + std::to_string(shadow_idx) + "]";
        _light->SetInt((shadow_name + ".shadow_map").c_str(), direction_shadow_delta_base + shadow_idx);
        _light->SetFM4((shadow_name + ".shadow_vp").c_str(), glm::value_ptr(d_light.second.vp));
        shadow_idx++;
      }
      idx++;
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

    renderQuad();
  }
  void Render::RenderSkyBox()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, _taa_jitter_fbo);
    _skybox->Use();
    _skybox->SetFM4("view", glm::value_ptr(_camera_view));
    _skybox->SetFM4("projection", glm::value_ptr(_camera_projection));
    _skybox->SetInt("skybox", 0);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _gbuffer_frame_buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _taa_jitter_fbo);
    glBlitFramebuffer(0, 0, _windows_width, _windows_height, 0, 0, _windows_width,
      _windows_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    if (!_enable_ibl)
    {
      return;
    }

    auto skybox_texture = GetTextureCubeResource(_pbr_texture_skybox);
    skybox_texture->BindToTexture(0);

    renderBox();
  }
  void Render::RenderTAA()
  {
    if (!_taa_jitter_idx) {
      // first render taa
      glBindFramebuffer(GL_READ_FRAMEBUFFER, _taa_jitter_fbo);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _taa_his_fbo);
      glBlitFramebuffer(0, 0, _windows_width, _windows_height, 0, 0, _windows_width,
                        _windows_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      // return;
    } else {
      // taa merge
      // glBindFramebuffer(GL_READ_FRAMEBUFFER, _gbuffer_frame_buffer);
      // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _taa_his_fbo);
      // glBlitFramebuffer(0, 0, _windows_width, _windows_height, 0, 0, _windows_width,
      //                   _windows_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

      glBindFramebuffer(GL_FRAMEBUFFER, _taa_his_fbo);
      glViewport(0, 0, _windows_width, _windows_height);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      auto taa_velocity_texture = GetTexture2DResource(_g_tta_velocity);
      _taa_sample->Use();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _taa_last_texture);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _taa_jitter_texture);
      taa_velocity_texture->BindToTexture(2);
      
      _taa_sample->SetInt("last_frame", 0);
      _taa_sample->SetInt("jitter_frame", 1);
      _taa_sample->SetInt("velocity", 2);
      _taa_sample->SetUInt("screen_width", _windows_width);
      _taa_sample->SetUInt("screen_height", _windows_height);
      _taa_sample->SetFloat("blend_ratio", _taa_blend_ratio);
      renderQuad();
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, _taa_his_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _taa_last_fbo);
    glBlitFramebuffer(0, 0, _windows_width, _windows_height, 0, 0, _windows_width,
                      _windows_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // copy output to history
  }
  void Render::RenderPost()
  {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _taa_his_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, _windows_width, _windows_height, 0, 0, _windows_width,
      _windows_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // RenderTAA();
  }
  void Render::ComputeClusterBox()
  {
    _cluster_init->Use();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _cluster_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _cluster_ssbo);

    _cluster_init->SetUInt("screen_width", _windows_width);
    _cluster_init->SetUInt("screen_height", _windows_height);

    _cluster_init->SetFloat("z_near", _z_near);
    _cluster_init->SetFloat("z_far", _z_far);
    _cluster_init->SetUInt("tile_size", _tile_size);
    _cluster_init->SetFM4("inverse_projection", glm::value_ptr(glm::inverse(
      glm::perspective(glm::radians(60.0f), float(_windows_width)/ _windows_height, _z_near, _z_far))));

    _cluster_init->Compute(_tile_x, _tile_y, _z_slices);
  }
  void Render::ComputeClusterLight()
  {
    _cluster_point_lights.resize(_point_light.size());
    int idx = 0;
    for (auto const& light : _point_light) {
      _cluster_point_lights[idx].diffuse = light.second.color;
      _cluster_point_lights[idx].position = light.second.position;
      _cluster_point_lights[idx].radius = light.second.radius;
      _cluster_point_lights[idx].shadow_idx = -1;
      idx++;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _point_light_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _cluster_point_lights.size() * sizeof(PLight), _cluster_point_lights.data(), GL_DYNAMIC_COPY);

    unsigned int init_index = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _global_index_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), &init_index, GL_DYNAMIC_COPY);

    _cluster_light->Use();
    _cluster_light->SetFM4("view", glm::value_ptr(_camera_view));

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _cluster_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _light_grid_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _point_light_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _point_light_idx_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, _global_index_ssbo);

    unsigned int sum_cluster = _tile_x * _tile_y * _z_slices;
    _cluster_light->Compute(1, (sum_cluster + 255) / 256, 4);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
    auto tta_velocity_texture = GetTexture2DResource(_g_tta_velocity);

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
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D,
      tta_velocity_texture->GetTexture(), 0);
    unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(6, attachments);
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
  void Render::InitCluster()
  {
    glGenBuffers(1, &_cluster_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _cluster_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _z_slices * _tile_x * _tile_y * sizeof(AABBBox), nullptr, GL_DYNAMIC_COPY);

    ComputeClusterBox();

    glGenBuffers(1, &_light_grid_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _light_grid_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _z_slices * _tile_x * _tile_y * sizeof(LightGrid), nullptr, GL_DYNAMIC_COPY);

    glGenBuffers(1, &_global_index_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _global_index_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);

    glGenBuffers(1, &_point_light_idx_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _point_light_idx_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1000000 * sizeof(unsigned int), nullptr, GL_DYNAMIC_COPY);

    glGenBuffers(1, &_point_light_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
    delete _cluster_init;
    delete _cluster_light;
    delete _taa_sample;

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
    _cluster_init = new Shader("shader/cluster_init_cs.glsl");
    _cluster_light = new Shader("shader/cluster_light_cs.glsl");
    _taa_sample = new Shader("shader/quad_sampler_vs.glsl", "shader/taa_sample.glsl");
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
    _g_tta_velocity = GenTexture2D(_windows_width, _windows_height, false, 2);
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

  void Render::InitTAA()
  {
    _taa_jitter_idx = 0;

    glGenRenderbuffers(1, &_taa_jitter_rbo);
    glGenFramebuffers(1, &_taa_jitter_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _taa_jitter_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _taa_jitter_fbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _windows_width, _windows_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _taa_jitter_rbo);

    _taa_jitter_texture = render::genTexture2D(_windows_width, _windows_height, false, 4);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           _taa_jitter_texture, 0);
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    // history frame
    glGenRenderbuffers(1, &_taa_his_rbo);
    glGenFramebuffers(1, &_taa_his_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _taa_his_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _taa_his_fbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _windows_width, _windows_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _taa_his_rbo);

    _taa_his_texture = render::genTexture2D(_windows_width, _windows_height, false, 4);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           _taa_his_texture, 0);
    glDrawBuffers(1, attachments);

    // last frame
    glGenFramebuffers(1, &_taa_last_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _taa_last_fbo);
    _taa_last_texture = render::genTexture2D(_windows_width, _windows_height, false, 4);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           _taa_last_texture, 0);
    glDrawBuffers(1, attachments);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
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
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    return res;
  }
}
