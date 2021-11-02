#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 WorldPos;
out vec2 TexCoords;
out mat3 TBN;

out vec3 ViewPos;
out mat3 TBN_View;

// TAA
uniform vec2 jitter;
uniform mat4 last_mvp;
out vec2 real_pos;
out vec2 last_pos;

void main () {
  vec4 world_pos = model * vec4(aPos, 1.0f);
  vec4 view_pos = view * world_pos;

  mat4 jitter_projection = projection;
  jitter_projection[3][0] = 2.0 * jitter.x * -view_pos.z;
  jitter_projection[3][1] = 2.0 * jitter.y * -view_pos.z;

  vec4 proj_pos_normal = projection * view_pos;
  vec4 proj_pos_jitter = jitter_projection * view_pos;
  vec4 last_proj_pos = last_mvp * vec4(aPos, 1.0f);

  WorldPos = world_pos.xyz;
  ViewPos = view_pos.xyz;
  TexCoords = aTex;

  gl_Position = proj_pos_jitter;
  real_pos = (proj_pos_normal.xy / proj_pos_normal.w) * 0.5 + 0.5;
  last_pos = (last_proj_pos.xy / last_proj_pos.w) * 0.5 + 0.5;

  vec3 T = normalize(aTangent);
  vec3 B = normalize(aBitangent);
  vec3 N = normalize(aNormal);
  TBN = mat3(transpose(inverse(model))) * mat3(T, B, N);
  TBN_View = mat3(transpose(inverse(view))) * TBN;
}
