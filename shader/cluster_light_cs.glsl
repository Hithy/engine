#version 430 core
layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct AABBBox {
  vec4 minPoint;
  vec4 maxPoint;
  vec4 minNearPoint;
  vec4 maxNearPoint;
};

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

layout(std430, binding = 1) buffer Cluster {
  AABBBox cluster[];
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

layout (std430, binding = 5) buffer globalIndexCountSSBO{
  uint globalIndexCount;
};

layout(std430, binding = 6) buffer FogSSBO {
  FogAccum fog_vbuffer[];
};

layout(std430, binding = 7) buffer FogSSBOOld {
  FogAccum fog_vbuffer_old[];
};

layout(std430, binding = 8) buffer FogNoiseSSBO {
  vec4 fog_noise[];
};

struct PLightShadow {
  samplerCube shadow_map;
};

const float PI = 3.14159265359;

uniform mat4 view;
uniform mat4 last_view;
uniform mat4 last_proj;
uniform vec3 cam_world_pos;
uniform PLightShadow point_light_shadow[3];
uniform uint frame_idx;

const uint tile_size = 8;
const uint z_slices = 64;
const uint tile_x = (1920 + tile_size - 1) / tile_size;
const uint tile_y = (1080 + tile_size - 1) / tile_size;

const float z_near = 0.1;
const float z_far = 30.0;

shared PLight batch_lights[64];

bool testRadiusAABB(vec3 min_point, vec3 max_point, vec3 center, float radius);
bool testLightAABB(uint light, uint tile);

vec3 cluster2view(vec3 cluster_coord) {
    vec2 ndc_xy = vec2(cluster_coord.xy / vec2(tile_x, tile_y)) * 2.0 - vec2(1.0, 1.0);
    vec4 ndc_pos = vec4(ndc_xy, -1.0, 1.0);
    vec4 view_pos_h = inverse(last_proj) * ndc_pos;
    vec3 view_dir = normalize(view_pos_h.xyz);
    float view_z = z_near * pow(z_far / z_near, float(cluster_coord.z) / float(z_slices));

    return view_dir * view_z;
}

void calc_fog(uint j, uint tile_index) {
    const vec3 fog_s = vec3(0.5, 1.0, 2.0);
    const float fog_g = 0.2;
    const float fog_t = 3.0;
    const vec3 fog_world_center = vec3(-15.0, -4.0, -10.0);
    const float fog_radius = 25.0f;

    uint delta_z = tile_index / (tile_x * tile_y);
    uint delta_y = (tile_index % (tile_x * tile_y)) / tile_x;
    uint delta_x = (tile_index % (tile_x * tile_y)) % tile_x;

    float depth_next = -z_near * pow(z_far / z_near, float(delta_z + 1) / float(z_slices));
    float depth_curr = -z_near * pow(z_far / z_near, float(delta_z) / float(z_slices));

    vec3 center_coord = vec3(delta_x + 0.5, delta_y + 0.5, delta_z + 0.5);
    vec3 cluster_view_center = cluster2view(center_coord);

    vec3 jitter_ratio = fog_noise[frame_idx % 7].xyz + vec3(0.0, 0.0, -0.5);
    // jitter_ratio = vec3(0.0, 0.0, 0);
    vec3 jitter_coord = center_coord + jitter_ratio;
    vec3 jitter_view_pos = cluster2view(jitter_coord);

    // vec3 jitter_ratio = fog_noise[frame_idx % 16].xyz;
    // jitter_ratio = vec3(0, 0, 0);

    // jitter = vec4(0,0,0,0);

    vec4 jitter_fog = vec4(0.0, 0.0, 0.0, 1.0);
    // sample_pos = cluster_center.xyz;
    // vec4 world_cluster_center = inverse(view) * vec4(cluster_center.xyz, 1.0);
    vec4 world_cluster_center = inverse(view) * vec4(cluster_view_center.xyz, 1.0);
    vec3 fog_sample_center = (inverse(view) * vec4(jitter_view_pos, 1.0)).xyz;
    vec3 fog_center = fog_world_center;

    vec4 last_view_pos = last_view * vec4(world_cluster_center.xyz, 1.0);
    vec4 last_clip_pos = last_proj * last_view_pos;
    vec2 offset = last_clip_pos.xy / last_clip_pos.w;
    bool outside = false;
    offset += 1.0;
    offset /= 2.0;
    if (offset.x < 0 || offset.y < 0 || offset.x > 1 || offset.y > 1) {
        outside = true;
    }
    
    offset *= vec2(tile_x, tile_y);
    uint last_x = uint(offset.x);
    uint last_y = uint(offset.y);
    float log_far_near = log(z_far / z_near);
    uint last_z = uint(log(length(last_view_pos)) * z_slices / log_far_near - z_slices * log(z_near) / log_far_near);
    uint last_idx = last_z * tile_x * tile_y + last_y * tile_x + last_x;
    float not_in_shadow = 1.0;
    if (length(fog_sample_center - fog_center) < fog_radius) {
        if (batch_lights[j].shadow_idx >= 0) {
            vec3 fragToLight = fog_sample_center - batch_lights[j].position;
            float currentDepth = length(fragToLight);
            float closestDepth = texture(point_light_shadow[batch_lights[j].shadow_idx].shadow_map, fragToLight).r;
            closestDepth *= 100.0f;
            if(currentDepth > closestDepth)
                not_in_shadow = 0.0;
        }

        vec3 light_pos = batch_lights[j].position;
        vec3 light_dir = fog_sample_center - light_pos;
        vec3 view_dir = cam_world_pos - fog_sample_center;
        float vLight = 1.0 / dot(light_dir, light_dir);
        float k = 1.55 * fog_g - 0.55 * fog_g * fog_g * fog_g;
        float cos_theta = dot(normalize(-view_dir), normalize(light_dir));
        float p = (1 - k * k) / (4 * PI * (1 + k * cos_theta) * (1 + k * cos_theta));
        jitter_fog.rgb += vec3(PI * batch_lights[j].diffuse * vLight * p * fog_s * not_in_shadow);

        float depth = depth_next - depth_curr;
        float transmittence = exp(-fog_t * depth);
        jitter_fog.a = transmittence;
        jitter_fog.a *= not_in_shadow;
        jitter_fog.a += (1.0 - not_in_shadow);
    }

    float ratio = (1 - 1/7.0);
    fog_vbuffer[tile_index].scatter_extinction = jitter_fog;

    vec4 last_color = fog_vbuffer_old[last_idx].scatter_extinction;
    if (!outside && frame_idx > 1){
        fog_vbuffer[tile_index].scatter_extinction = mix(jitter_fog, last_color, ratio);
    }
}


void main() {
  // uint tile_index = gl_LocalInvocationIndex * gl_NumWorkGroups.x * gl_NumWorkGroups.y + \
  //                    gl_WorkGroupID.y * gl_NumWorkGroups.x + \
  //                    gl_WorkGroupID.x;

  uint thread_count = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
  uint tile_index = gl_LocalInvocationIndex +                           \
                    thread_count * gl_NumWorkGroups.x * gl_NumWorkGroups.y *  gl_WorkGroupID.z + \
                    thread_count * gl_NumWorkGroups.x * gl_WorkGroupID.y + \
                    thread_count * gl_WorkGroupID.x;

  uint visibleLightCount = 0;
  uint visibleLightIndices[1000];
  uint light_count = point_lights.length();

  uint batch_count = (light_count + thread_count - 1) / thread_count;
  barrier();
  for (uint i=0; i<=batch_count; i++) {
    uint light_idx = i * thread_count + gl_LocalInvocationIndex;
    light_idx = min(light_idx, light_count - 1);

    batch_lights[gl_LocalInvocationIndex] = point_lights[light_idx];
    barrier();

    for (uint j=0; j<thread_count; j++) {
      light_idx = i * thread_count + j;
      if (light_idx >= light_count) {
        break;
      }
      if (testLightAABB(j, tile_index)) {
        visibleLightIndices[visibleLightCount] = light_idx;
        visibleLightCount++;
      }
      calc_fog(j, tile_index);
    }
  }

  uint offset = atomicAdd(globalIndexCount, visibleLightCount);
  for (uint i=0; i<visibleLightCount; i++) {
    point_light_index[offset + i] = visibleLightIndices[i];
  }

  light_grids[tile_index].offset = offset;
  light_grids[tile_index].count = visibleLightCount;
}

bool testRadiusAABB(vec3 min_point, vec3 max_point, vec3 center, float radius) {
  float sum = 0.0;
  for (uint i=0; i<3; i++) {
    float l = center[i];
    float minp = min_point[i];
    float maxp = max_point[i];

    if (l < minp) {
      sum += (minp - l) * (minp - l);
    } else if (l > maxp) {
      sum += (l - maxp) * (l - maxp);
    }
  }

  return sum <= (radius * radius);
}

bool testLightAABB(uint light, uint tile) {
  vec3 light_pos = (view * vec4(batch_lights[light].position, 1.0)).xyz;
  float light_radius = batch_lights[light].radius;

  vec3 min_point = cluster[tile].minPoint.xyz;
  vec3 max_point = cluster[tile].maxPoint.xyz;

  return testRadiusAABB(min_point, max_point, light_pos, light_radius);
}
