#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

// pass1: shadow for each light
// pass2: gbuffer
// pass3: ssao
// pass4: lighting

namespace render {
  class Shader;
  class Model;

  struct RenderItem {
    uint64_t obj_id;
    glm::mat4 transform;
    uint64_t mesh;
    uint64_t albedo;
    uint64_t normal;
    uint64_t metalic;
    uint64_t roughness;
    uint64_t ao;
  };

  struct RenderPointLight {
    uint64_t light_id;
    glm::vec3 position;
    glm::vec3 color;
  };

  struct RenderDirectionLight {
    uint64_t light_id;
    glm::vec3 direction;
    glm::vec3 color;
  };

  class Render {
  public:
    static Render& GetInstance() {
      static Render inst;
      return inst;
    }

    void PrepareRender();
    void DoRender();
    void Init();

  public:
    // must invoke
    void SetPbrSkyBox(const char* path);
    void SetCameraTrans(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& pos);

  public:
    // modify
    void AddRenderItem(const RenderItem& new_item);
    void DelRenderItem(uint64_t obj_id);
    void ClearRenderItem();

    void AddPointLight(const RenderPointLight& new_light);
    void DelPointLight(uint64_t light_id);
    void ClearPointLight();

    void AddDirectionLight(const RenderDirectionLight& new_light);
    void DelDirectionLight(uint64_t light_id);
    void ClearDirectionLight();

  private:
    Render();

    // render
    void RenderGbuffer();
    void RenderLight();
    void RenderSkyBox();

    // init
    void InitPbrRenderBuffer();
    void InitPbrSkybox();
    void InitPbrIrradiance();
    void InitPbrPrefilter();
    void InitPbrBrdf();

    void InitShader();
    void InitObjects();
    void InitPBR();

  private:
    // shader
    Shader* _pbr_hdr_preprocess;
    Shader* _pbr_irradiance;
    Shader* _pbr_prefilter;
    Shader* _pbr_brdf;

    Shader* _gbuffer;
    Shader* _light;
    Shader* _skybox;

    // pbr init texture
    uint64_t _pbr_texture_skybox;
    uint64_t _pbr_texture_irradiance;
    uint64_t _pbr_texture_prefilter;
    uint64_t _pbr_texture_brdf;

    unsigned int _pbr_frame_buffer;
    unsigned int _pbr_render_buffer;

    unsigned int _gbuffer_frame_buffer;
    unsigned int _gbuffer_render_buffer;

    // gbuffer
    uint64_t _g_position_ao;
    uint64_t _g_albedo_roughness;
    uint64_t _g_normal_metalic;

    // active camera
    glm::mat4 _camera_view;
    glm::mat4 _camera_projection;
    glm::vec3 _camera_pos;

    // objs to render
    std::unordered_map<uint64_t, RenderItem> _render_objects;

    // light
    std::unordered_map<uint64_t, RenderPointLight> _point_light;
    std::unordered_map<uint64_t, RenderDirectionLight> _direction_light;

  private:
    std::string _pbr_skybox_path;

  private:
    // config
    int _windows_width;
    int _windows_height;

    int _pbr_skybox_width;
    int _pbr_skybox_height;

    int _pbr_irradiance_width;
    int _pbr_irradiance_height;

    int _pbr_prefilter_width;
    int _pbr_prefilter_height;

    int _pbr_brdf_width;
    int _pbr_brdf_height;
  };
}