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
    // interface
    uint64_t light_id;
    glm::vec3 position;
    glm::vec3 color;
    bool enable_shadow;

    // inner
    std::vector<glm::mat4> vps;
    int shadow_map_idx;
  };

  struct RenderDirectionLight {
    // interface
    uint64_t light_id;
    glm::vec3 direction;
    glm::vec3 color;
    bool enable_shadow;

    // inner
    int shadow_map_idx;
    glm::mat4 vp;
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
    void RenderShadow();
    void RenderGbuffer();
    void RenderSSAO();
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
    void InitShadowMap();
    void InitSSAO();

  private:
    unsigned int GenShadowMap(int light_type);

  private:
    // shader
    Shader* _pbr_hdr_preprocess;
    Shader* _pbr_irradiance;
    Shader* _pbr_prefilter;
    Shader* _pbr_brdf;

    Shader* _gbuffer;
    Shader* _ssao;
    Shader* _light;
    Shader* _skybox;
    Shader* _shadow_shader_point;
    Shader* _shadow_shader_direction;

    // pbr init texture
    uint64_t _pbr_texture_skybox;
    uint64_t _pbr_texture_irradiance;
    uint64_t _pbr_texture_prefilter;
    uint64_t _pbr_texture_brdf;

    unsigned int _pbr_frame_buffer;
    unsigned int _pbr_render_buffer;

    unsigned int _gbuffer_frame_buffer;
    unsigned int _gbuffer_render_buffer;

    unsigned int _shadow_frame_buffer;
    unsigned int _shadow_render_buffer;

    // ssao
    unsigned int _ssao_map;
    unsigned int _ssao_noise_map;
    unsigned int _ssao_frame_buffer;
    unsigned int _ssao_render_buffer;
    std::vector<glm::vec3> _ssao_kernal;

    // gbuffer
    uint64_t _g_position_ao;
    uint64_t _g_albedo_roughness;
    uint64_t _g_normal_metalic;
    uint64_t _g_view_position;
    uint64_t _g_view_normal;

    // active camera
    glm::mat4 _camera_view;
    glm::mat4 _camera_projection;
    glm::vec3 _camera_pos;

    // objs to render
    std::unordered_map<uint64_t, RenderItem> _render_objects;

    // light
    std::unordered_map<uint64_t, RenderPointLight> _point_light;
    std::unordered_map<uint64_t, RenderDirectionLight> _direction_light;

    int _point_shadow_count;
    std::vector<unsigned int> _point_shadow_map;
    int _diretion_shadow_count;
    std::vector<unsigned int> _diretion_shadow_map;

  private:
    std::string _pbr_skybox_path;
    bool _enable_ibl;

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

    int _shadow_map_width;
    int _shadow_map_height;
    int _max_point_light_shadow;
    int _max_direction_light_shadow;

  private:
    bool _enable_shadow;
    bool _enable_ssao;

  private:
    float _dt_shadow_pass;
    float _dt_gbuffer_pass;
    float _dt_ssao_pass;
    float _dt_light_pass;
    float _dt_skybox_pass;
    float _dt_imgui_pass;

  };
}