#version 430 core

const float PI = 3.14159265359;

struct PLight {
  vec3 position;
  int shadow_idx;
  vec3 diffuse;
  float radius;
};

struct LightGrid {
  uint offset;
  uint count;
};

layout(std430, binding = 2) buffer LightGridList
{
  LightGrid light_grids[];
};

layout(std430, binding = 3) buffer PointLightList {
  PLight point_lights[];
};

layout(std430, binding = 4) buffer LightIndexList {
  uint point_light_index[];
};

struct PLightShadow {
  samplerCube shadow_map;
};

struct DLight {
  vec3 direction;
  vec3 diffuse;
  int shadow_idx;
};

struct DLightShadow {
  mat4 shadow_vp;
  sampler2D shadow_map;
};

#define POINT_LIGHT_MAX_COUNT 1000
uniform int point_light_count;
uniform PLight point_light_list[POINT_LIGHT_MAX_COUNT];

#define DIRECTION_LIGHT_MAX_COUNT 100
uniform int direction_light_count;
uniform DLight direction_light_list[DIRECTION_LIGHT_MAX_COUNT];

uniform PLightShadow point_light_shadow[3];
uniform DLightShadow direction_light_shadow[3];

// viewer
uniform vec3 cam_pos;
uniform mat4 cam_view;

//IBL
uniform samplerCube irradiance_map;
uniform samplerCube prefilter_map;
uniform sampler2D brdf_lut;

// gbuffer
uniform sampler2D gPosAO;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gNormalMetalic;
uniform sampler2D gSSAO;

// switch
uniform int enable_ssao;
uniform int enable_shadow;
uniform int enable_ibl;

// cluster
uniform uint screen_width;
uniform uint screen_height;
uniform uint tile_size;
uniform float z_near;
uniform float z_far;
uniform uint z_slices;
uniform uint tile_x;
uniform uint tile_y;

in vec2 TexCoords;
out vec4 FragColor;

// pbr
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

// shadow
float CalcDirShadow(DLight light, vec3 pos, vec3 normal);
float CalcPointShadow(PLight light, vec3 world_pos);

void main() {
  vec4 pos_ao = texture(gPosAO, TexCoords);
  vec4 albedo_roughness = texture(gAlbedoRoughness, TexCoords);
  vec4 normal_metalic = texture(gNormalMetalic, TexCoords);
  float ssao = 1.0;
  if (enable_ssao > 0) {
    ssao = texture(gSSAO, TexCoords).r;
  }
  float roughness = albedo_roughness.a;
  vec3 albedo = pow(albedo_roughness.rgb, vec3(2.2)) * ssao;

  vec3 N = normal_metalic.rgb;
  float metallic = normal_metalic.a;

  float ao = pos_ao.a;
  vec3 WorldPos = pos_ao.rgb;

  vec3 V = normalize(cam_pos - WorldPos);
  vec3 R = reflect(-V, N);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 sum_color = vec3(0.0);

  vec3 view_pos = (cam_view * vec4(WorldPos, 1.0)).xyz;

  vec2 screen_pos = TexCoords * vec2(screen_width, screen_height);
  uvec2 cluster_xy = uvec2(screen_pos / float(tile_size));

  float log_far_near = log(z_far / z_near);
  uint cluster_z = uint(log(-view_pos.z) * z_slices / log_far_near - z_slices * log(z_near) / log_far_near);
  // cluster_z = 6;
  uint cluster_idx = cluster_z * tile_x * tile_y + cluster_xy.y * tile_x + cluster_xy.x;

  for (uint i=0; i<light_grids[cluster_idx].count; i++) {
    vec3 light_pos = point_lights[point_light_index[light_grids[cluster_idx].offset + i]].position;
    float light_radius = point_lights[point_light_index[light_grids[cluster_idx].offset + i]].radius;
    vec3 light_color = point_lights[point_light_index[light_grids[cluster_idx].offset + i]].diffuse;

    vec3 L = normalize(light_pos - WorldPos);
    vec3 H = normalize(L + V);

    float dist = length(light_pos - WorldPos);
    if (dist > light_radius) {
      continue;
    }
    float attenuation = 1.0 - (dist / light_radius) * (dist / light_radius);
    vec3 radiance = light_color * attenuation;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    
    vec3 F = fresnelSchlick(max(0.0, dot(V, H)), F0);
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float VdotN = max(0.0, dot(V, N));
    float LdotN = max(0.0, dot(L, N));

    vec3 numerator = NDF * G * F;
    float denominator = 4 * VdotN * LdotN + 0.0001;

    vec3 inner = (kD * albedo / PI) + numerator / denominator;
    vec3 LO = inner * radiance * LdotN;

    float shadow_ratio = 0.0;
    if (enable_shadow > 0) {
      shadow_ratio = CalcPointShadow(point_lights[point_light_index[light_grids[cluster_idx].offset + i]], WorldPos);
    }

    sum_color += LO * (1.0 - shadow_ratio);
  }

  // direction light
  for (int i=0; i<direction_light_count; i++) {
    vec3 L = normalize(-direction_light_list[i].direction);
    vec3 H = normalize(L + V);

    vec3 radiance = direction_light_list[i].diffuse;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    
    vec3 F = fresnelSchlick(max(0.0, dot(V, H)), F0);
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float VdotN = max(0.0, dot(V, N));
    float LdotN = max(0.0, dot(L, N));

    vec3 numerator = NDF * G * F;
    float denominator = 4 * VdotN * LdotN + 0.0001;

    vec3 inner = (kD * albedo / PI) + numerator / denominator;
    vec3 LO = inner * radiance * LdotN;

    float shadow_ratio = 0.0;
    if (enable_shadow > 0) {
      shadow_ratio = CalcDirShadow(direction_light_list[i], WorldPos, N);
    }

    sum_color += LO * (1.0 - shadow_ratio);
  }

  vec3 ambient = vec3(0.0);
  vec3 specular = vec3(0.0);

  if (enable_ibl > 0) {
    // IBL
    vec3 F = fresnelSchlickRoughness(max(0.0, dot(V, N)), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradiance_map, N).rgb;
    ambient = kD * irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilter = textureLod(prefilter_map, R,  roughness * MAX_REFLECTION_LOD).rgb;   
    float NdotV = max(0.0, dot(N, V));
    vec2 brdf = texture(brdf_lut, vec2(NdotV, roughness)).rg;
    specular = prefilter * (F * brdf.x + brdf.y);
  }
  
  vec3 color = sum_color + (ambient + specular) * ao;
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;

  return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(0.0, dot(N, V));
  float NdotL = max(0.0, dot(N, L));

  return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;

  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float tmp = (NdotH2 * (a2 - 1.0) + 1.0);

  return a2 / (PI * tmp * tmp);
}

float CalcDirShadow(DLight light, vec3 pos, vec3 normal)
{
  float shadow_ratio = 0.0f;
  if (light.shadow_idx >= 0) {
    vec4 shadow_tex = direction_light_shadow[light.shadow_idx].shadow_vp * vec4(pos, 1.0f);
    vec3 projCoords = shadow_tex.xyz / shadow_tex.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec2 texelSize = 1.0 / textureSize(direction_light_shadow[light.shadow_idx].shadow_map, 0);
    float bias = max(0.05 * (1.0 - pow(dot(normal, normalize(-light.direction)), 2.0)), 0.005);
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(direction_light_shadow[light.shadow_idx].shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow_ratio += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow_ratio /= 9.0;
  }

  return shadow_ratio;
}

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float CalcPointShadow(PLight light, vec3 world_pos)
{
  float shadow_ratio = 0.0;

  if (light.shadow_idx >= 0) {
    vec3 fragToLight = world_pos - light.position;
    float currentDepth = length(fragToLight);

    float bias  = 0.005; 
    int samples = 20;

    float viewDistance = length(cam_pos - world_pos);
    float diskRadius = (1.0 + (viewDistance / 50.0f)) / 25.0;

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(point_light_shadow[light.shadow_idx].shadow_map, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= 50.0f;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow_ratio += 1.0;
    }
    shadow_ratio /= float(samples);
  }

  return shadow_ratio;
}
