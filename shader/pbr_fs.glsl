#version 330 core

const float PI = 3.14159265359;

struct PLight {
  vec3 position;
  vec3 diffuse;
  int shadow_map_idx;
};

struct DLight {
  vec3 direction;
  vec3 diffuse;
  mat4 shadow_vp;
  int shadow_map_idx;
};

#define POINT_LIGHT_MAX_COUNT 10
uniform int point_light_count;
uniform PLight point_light_list[POINT_LIGHT_MAX_COUNT];
uniform samplerCube point_light_shadow[5];

#define DIRECTION_LIGHT_MAX_COUNT 10
uniform int direction_light_count;
uniform DLight direction_light_list[DIRECTION_LIGHT_MAX_COUNT];
uniform sampler2D direction_light_shadow[5];

// viewer
uniform vec3 cam_pos;

//IBL
uniform samplerCube irradiance_map;
uniform samplerCube prefilter_map;
uniform sampler2D brdf_lut;

// gbuffer
uniform sampler2D gPosAO;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gNormalMetalic;

in vec2 TexCoords;
out vec4 FragColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

float CalcDirShadow(DLight light, vec3 pos, vec3 normal)
{
  float shadow_ratio = 0.0f;
  if (light.shadow_map_idx >= 0) {
    vec4 shadow_tex = light.shadow_vp * vec4(pos, 1.0f);
    vec3 projCoords = shadow_tex.xyz / shadow_tex.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec2 texelSize = 1.0 / textureSize(direction_light_shadow[light.shadow_map_idx], 0);
    float bias = max(0.05 * (1.0 - pow(dot(normal, normalize(-light.direction)), 2.0)), 0.005);
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(direction_light_shadow[light.shadow_map_idx], projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow_ratio += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow_ratio /= 9.0;

    /*
    float pcfDepth = texture(direction_light_shadow[light.shadow_map_idx], projCoords.xy).r; 
    shadow_ratio += projCoords.z > pcfDepth ? 1.0 : 0.0;     
    */
  }

  return shadow_ratio;
}

void main() {
  vec4 pos_ao = texture(gPosAO, TexCoords);
  vec4 albedo_roughness = texture(gAlbedoRoughness, TexCoords);
  vec4 normal_metalic = texture(gNormalMetalic, TexCoords);

  float roughness = albedo_roughness.a;
  vec3 albedo = pow(albedo_roughness.rgb, vec3(2.2));

  vec3 N = normal_metalic.rgb;
  float metallic = normal_metalic.a;

  float ao = pos_ao.a;
  vec3 WorldPos = pos_ao.rgb;

  vec3 V = normalize(cam_pos - WorldPos);
  vec3 R = reflect(-V, N);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 sum_color = vec3(0.0);

  // point light
  for (int i=0; i<point_light_count; i++) {
    PLight light = point_light_list[i];
    vec3 L = normalize(light.position - WorldPos);
    vec3 H = normalize(L + V);

    vec3 light_color = light.diffuse;
    float dist = length(light.position - WorldPos);
    float attenuation = 1.0 / (dist * dist);
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

    if (light.shadow_map_idx >= 0) {
      float shadow_depth = texture(point_light_shadow[light.shadow_map_idx], -L).r * 100.0f;
      float bias = 0.05;
      if (shadow_depth + bias < dist) {
        LO *= 0.0;
      }
    }

    sum_color += LO;
  }

  // direction light
  for (int i=0; i<direction_light_count; i++) {
    DLight light = direction_light_list[i];
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(L + V);

    vec3 radiance = light.diffuse;

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

    float shadow_ratio = CalcDirShadow(light, WorldPos, N);

    sum_color += LO * (1.0 - shadow_ratio);
  }

  vec3 ambient = vec3(0.0);
  vec3 specular = vec3(0.0);

  {
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
