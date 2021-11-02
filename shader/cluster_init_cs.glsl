#version 430 core
layout (local_size_x = 1, local_size_y = 1) in;

struct AABBBox {
  vec4 minPoint;
  vec4 maxPoint;
};

layout(std430, binding = 1) buffer Cluster
{
  AABBBox cluster[];
};

uniform uint screen_width;
uniform uint screen_height;

uniform float z_near;
uniform float z_far;
uniform uint tile_size;

uniform mat4 inverse_projection;

vec3 screen2view(uvec2 screen_pos);

void main() {
  uint cluster_idx = gl_WorkGroupID.z * gl_NumWorkGroups.x * gl_NumWorkGroups.y + \
                     gl_WorkGroupID.y * gl_NumWorkGroups.x + \
                     gl_WorkGroupID.x;

  uvec2 screen_pos_min = gl_WorkGroupID.xy * tile_size;
  uvec2 screen_pos_max = (gl_WorkGroupID.xy + uvec2(1, 1)) * tile_size;

  vec3 view_pos_min = screen2view(screen_pos_min);
  vec3 view_pos_max = screen2view(screen_pos_max);

  float z_view_front = -z_near * pow(z_far / z_near, float(gl_WorkGroupID.z) / float(gl_NumWorkGroups.z));
  float z_view_back = -z_near * pow(z_far / z_near, float(gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));

  vec3 min_front = vec3(view_pos_min.xy * (z_view_front / view_pos_min.z), z_view_front);
  vec3 min_back = vec3(view_pos_min.xy * (z_view_back / view_pos_min.z), z_view_back);
  vec3 max_front = vec3(view_pos_max.xy * (z_view_front / view_pos_max.z), z_view_front);
  vec3 max_back = vec3(view_pos_max.xy * (z_view_back / view_pos_max.z), z_view_back);

  cluster[cluster_idx].minPoint = vec4(min(min(min_front, min_back), min(max_front, max_back)), 1.0);
  cluster[cluster_idx].maxPoint = vec4(max(max(min_front, min_back), max(max_front, max_back)), 1.0);
}

vec3 screen2view(uvec2 screen_pos) {
  vec4 ndc_pos = vec4(vec2(screen_pos) * 2.0 / vec2(float(screen_width), float(screen_height)) - vec2(1.0, 1.0), -1.0, 1.0);
  vec4 view_pos_h = inverse_projection * ndc_pos;

  return view_pos_h.xyz / view_pos_h.w;
}
