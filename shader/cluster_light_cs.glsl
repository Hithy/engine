#version 430 core
layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct AABBBox {
  vec4 minPoint;
  vec4 maxPoint;
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

uniform mat4 view;

shared PLight batch_lights[64];

bool testLightAABB(uint light, uint tile);

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
    }
  }

  uint offset = atomicAdd(globalIndexCount, visibleLightCount);
  for (uint i=0; i<visibleLightCount; i++) {
    point_light_index[offset + i] = visibleLightIndices[i];
  }

  light_grids[tile_index].offset = offset;
  light_grids[tile_index].count = visibleLightCount;
}

bool testLightAABB(uint light, uint tile) {
  vec3 light_pos = (view * vec4(batch_lights[light].position, 1.0)).xyz;
  float light_radius = batch_lights[light].radius;

  vec3 min_point = cluster[tile].minPoint.xyz;
  vec3 max_point = cluster[tile].maxPoint.xyz;

  float sum = 0.0;
  for (uint i=0; i<3; i++) {
    float l = light_pos[i];
    float minp = min_point[i];
    float maxp = max_point[i];

    if (l < minp) {
      sum += (minp - l) * (minp - l);
    } else if (l > maxp) {
      sum += (l - maxp) * (l - maxp);
    }
  }

  return sum <= (light_radius * light_radius);
}
