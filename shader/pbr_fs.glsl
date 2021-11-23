#version 330 core

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

struct FogAccum {
  vec4 scatter_extinction;
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

layout(std430, binding = 6) buffer FogSSBO {
  FogAccum fog_vbuffer[];
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

#define DIRECTION_LIGHT_MAX_COUNT 1000
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
uniform int enable_poisson;

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

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 DebugColor;

// pbr
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

// shadow
float CalcDirShadow(DLight light, vec3 pos, vec3 normal);
float CalcPointShadow(PLight light, vec3 world_pos, float LDotN);

float InScatter(vec3 start, vec3 rd, vec3 lightPos, float d)
{
    vec3 q = start - lightPos;
    float b = dot(rd, q);
    float c = dot(q, q);
    float iv = 1.0f / max(0.00001, sqrt(c - b*b));
    float l = iv * (atan( (d + b) * iv) - atan( b*iv ));

    return l;
}

float raymarch(vec3 start, vec3 target, PLight light)
{
    const int stepNum= 500;
    vec3 ray = target - start;
    vec3 rd = normalize(ray);
    float stepSize= 30.0 / stepNum;
    vec3 lightPos = light.position;
    float t = 1.0; 
    float res = 0.0;

    int i=0;
    for(; i<stepNum;i++)
    {
        vec3 delta = t * rd;
        if (length(delta) > length(ray)) {
          break;
        }
        vec3 p = start + delta;
        float in_shadow = 0.0;
        if (light.shadow_idx >= 0) {
          // shadow
          vec3 fragToLight = p - light.position;
          float currentDepth = length(fragToLight);
          float closestDepth = texture(point_light_shadow[light.shadow_idx].shadow_map, fragToLight).r;
          closestDepth *= 100.0f;
          if(currentDepth > closestDepth)
              in_shadow = 1.0;
        }
        float vLight = 1.0 / dot(p-lightPos,p-lightPos);
        res += vLight * (1.0 - in_shadow) * stepSize;
        t += stepSize;
    }
    return res;
}

vec4 calcFog(uint x, uint y, uint max_z, vec2 screen_pos, float view_costheta) {
  vec4 accume = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 accume_right = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 accume_up = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 accume_up_right = vec4(0.0, 0.0, 0.0, 1.0);

  for (uint z=0; z<=min(max_z, z_slices - 1); z++) {
    vec4 sample_vec = fog_vbuffer[z * tile_x * tile_y + y * tile_x + x].scatter_extinction;
//    vec4 sample_vec_right = fog_vbuffer[z * tile_x * tile_y + y * tile_x + min(x + 1, tile_x - 1)].scatter_extinction;
//    vec4 sample_vec_up = fog_vbuffer[z * tile_x * tile_y + min(y + 1, tile_y - 1) * tile_x + x].scatter_extinction;
//    vec4 sample_vec_up_right = fog_vbuffer[z * tile_x * tile_y + min(y + 1, tile_y - 1) * tile_x + min(x + 1, tile_x - 1)].scatter_extinction;
//    float step_next = -z_near * pow(z_far / z_near, float(z+1) / float(z_slices));
//    float step_curr = -z_near * pow(z_far / z_near, float(z) / float(z_slices));
//    float step_len = abs(step_next - step_curr);

    accume.a *= accume.a;
    accume.rgb += sample_vec.rgb * accume.a;

//    accume_right.a *= sample_vec_right.a;
//    accume_right.rgb += sample_vec_right.rgb * accume_right.a;
//
//    accume_up.a *= sample_vec_up.a;
//    accume_up.rgb += sample_vec_up.rgb * accume_up.a;
//
//    accume_up_right.a *= sample_vec_up_right.a;
//    accume_up_right.rgb += sample_vec_up_right.rgb * accume_up_right.a;
  }

  return accume;
//
//
//  vec4 accume_top = mix(accume_up, accume_up_right, float(screen_pos.x - x * tile_size) / tile_size);
//  vec4 accume_bottom = mix(accume, accume_right, float(screen_pos.x - x * tile_size) / tile_size);
//  return mix(accume_bottom, accume_top, float(screen_pos.y - y * tile_size) / tile_size);
}

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
  vec3 rd = -V;

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 sum_color = vec3(0.0);

  vec3 view_pos = (cam_view * vec4(WorldPos, 1.0)).xyz;
  float view_costhera = dot(normalize(view_pos), vec3(0, 0, -1));

  vec2 screen_pos = TexCoords * vec2(screen_width, screen_height);
  uvec2 cluster_xy = uvec2(screen_pos / float(tile_size));

  float log_far_near = log(z_far / z_near);
  uint cluster_z = uint(log(-view_pos.z) * z_slices / log_far_near - z_slices * log(z_near) / log_far_near);
  uint cluster_idx = cluster_z * tile_x * tile_y + cluster_xy.y * tile_x + cluster_xy.x;

  float fog_log_far_near = log(30.0 / z_near);
  uint fog_cluster_z = uint(log(length(view_pos)) * z_slices / fog_log_far_near - z_slices * log(z_near) / fog_log_far_near);

  vec3 fog_color = vec3(0.0);
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
      shadow_ratio = CalcPointShadow(point_lights[point_light_index[light_grids[cluster_idx].offset + i]], WorldPos, LdotN);
    }

    // fog_color += (1.0 - shadow_ratio) * light_color * max(0.0, InScatter(cam_pos, rd, light_pos, min(100.0, length(cam_pos - WorldPos))));
    // fog_color += light_color * raymarch(cam_pos, WorldPos, point_lights[point_light_index[light_grids[cluster_idx].offset + i]]);
    vec4 fog_value = calcFog(cluster_xy.x, cluster_xy.y, fog_cluster_z, screen_pos, view_costhera);
    fog_color += fog_value.xyz;//vec3(max(0.0, fog_value.x),max(0.0, fog_value.y),max(0.0, fog_value.z));
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
  
  vec3 color = sum_color + (ambient + specular) * ao + fog_color;
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));
  //color = vec3((cluster_z / 4) / 64.0, 0, 0);
  FragColor = vec4(color, 1.0);
  DebugColor = vec4(fog_color, 1.0);
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

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

vec2 poissonDisk1[64] = vec2[](
    vec2(-0.5119625, -0.4827938),
    vec2(-0.2171264, -0.4768726),
    vec2(-0.7552931, -0.2426507),
    vec2(-0.7136765, -0.4496614),
    vec2(-0.5938849, -0.6895654),
    vec2(-0.3148003, -0.7047654),
    vec2(-0.42215, -0.2024607),
    vec2(-0.9466816, -0.2014508),
    vec2(-0.8409063, -0.03465778),
    vec2(-0.6517572, -0.07476326),
    vec2(-0.1041822, -0.02521214),
    vec2(-0.3042712, -0.02195431),
    vec2(-0.5082307, 0.1079806),
    vec2(-0.08429877, -0.2316298),
    vec2(-0.9879128, 0.1113683),
    vec2(-0.3859636, 0.3363545),
    vec2(-0.1925334, 0.1787288),
    vec2(0.003256182, 0.138135),
    vec2(-0.8706837, 0.3010679),
    vec2(-0.6982038, 0.1904326),
    vec2(0.1975043, 0.2221317),
    vec2(0.1507788, 0.4204168),
    vec2(0.3514056, 0.09865579),
    vec2(0.1558783, -0.08460935),
    vec2(-0.0684978, 0.4461993),
    vec2(0.3780522, 0.3478679),
    vec2(0.3956799, -0.1469177),
    vec2(0.5838975, 0.1054943),
    vec2(0.6155105, 0.3245716),
    vec2(0.3928624, -0.4417621),
    vec2(0.1749884, -0.4202175),
    vec2(0.6813727, -0.2424808),
    vec2(-0.6707711, 0.4912741),
    vec2(0.0005130528, -0.8058334),
    vec2(0.02703013, -0.6010728),
    vec2(-0.1658188, -0.9695674),
    vec2(0.4060591, -0.7100726),
    vec2(0.7713396, -0.4713659),
    vec2(0.573212, -0.51544),
    vec2(-0.3448896, -0.9046497),
    vec2(0.1268544, -0.9874692),
    vec2(0.7418533, -0.6667366),
    vec2(0.3492522, 0.5924662),
    vec2(0.5679897, 0.5343465),
    vec2(0.5663417, 0.7708698),
    vec2(0.7375497, 0.6691415),
    vec2(0.2271994, -0.6163502),
    vec2(0.2312844, 0.8725659),
    vec2(0.4216993, 0.9002838),
    vec2(0.4262091, -0.9013284),
    vec2(0.2001408, -0.808381),
    vec2(0.149394, 0.6650763),
    vec2(-0.09640376, 0.9843736),
    vec2(0.7682328, -0.07273844),
    vec2(0.04146584, 0.8313184),
    vec2(0.9705266, -0.1143304),
    vec2(0.9670017, 0.1293385),
    vec2(0.9015037, -0.3306949),
    vec2(-0.5085648, 0.7534177),
    vec2(0.9055501, 0.3758393),
    vec2(0.7599946, 0.1809109),
    vec2(-0.2483695, 0.7942952),
    vec2(-0.4241052, 0.5581087),
    vec2(-0.1020106, 0.6724468));

float CalcDirShadow(DLight light, vec3 pos, vec3 normal)
{
  float shadow_ratio = 0.0f;
  if (light.shadow_idx >= 0) {
    vec4 projCoords = direction_light_shadow[light.shadow_idx].shadow_vp * vec4(pos, 1.0f);
    projCoords = projCoords * 0.5 + 0.5;

    float bias = max(0.015 * (1.0 - dot(normal, normalize(-light.direction))), 0.001);
    // shadow_ratio = texture(direction_light_shadow[light.shadow_idx].shadow_map, vec3(projCoords.xy, projCoords.z - bias));
    
    vec2 texelSize = 1.0 / textureSize(direction_light_shadow[light.shadow_idx].shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(direction_light_shadow[light.shadow_idx].shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow_ratio += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }

    shadow_ratio /= 9.0;

    // float pcfDepth = texture(direction_light_shadow[light.shadow_idx].shadow_map, projCoords.xy).r; 
    // shadow_ratio += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
        shadow_ratio = 0.0;

    float visibility = 0.0;
    for (int i=0;i<4;i++) {
      if ( texture(direction_light_shadow[light.shadow_idx].shadow_map, projCoords.xy + poissonDisk[i] / 700.0 ).r  <  projCoords.z-bias ){
        // visibility -= (1.0 / 64.0);
        visibility += 0.2;
      }
    }

    shadow_ratio *= visibility;
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

float CalcPointShadow(PLight light, vec3 world_pos, float LDotN)
{
  float shadow_ratio = 0.0;

  if (light.shadow_idx >= 0) {
    vec3 fragToLight = world_pos - light.position;
    float currentDepth = length(fragToLight);

    float bias = max(0.5 * (1.0 - LDotN), 0.1);
    // float bias = 0.15;
    int samples = 20;

    float viewDistance = length(cam_pos - world_pos);
    float diskRadius = (1.0 + (viewDistance / 100.0f)) / 25.0;

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(point_light_shadow[light.shadow_idx].shadow_map, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= 100.0f;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow_ratio += 1.0;
    }
    shadow_ratio /= float(samples);
  }

  return shadow_ratio;
}
