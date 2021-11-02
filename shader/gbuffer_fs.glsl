#version 330 core

layout (location = 0) out vec4 gPosAO;
layout (location = 1) out vec4 gAlbedoRoughness;
layout (location = 2) out vec4 gNormalMetalic;
layout (location = 3) out vec3 gPosView;
layout (location = 4) out vec3 gNorView;
layout (location = 5) out vec2 gVelocity;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D metalic;
uniform sampler2D roughness;
uniform sampler2D ao;

uniform mat4 model;

in vec3 WorldPos;
in vec2 TexCoords;
in mat3 TBN;

in vec3 ViewPos;
in mat3 TBN_View;
in vec2 real_pos;
in vec2 last_pos;

void main() {
  gPosAO.rgb = WorldPos;
  gPosAO.a = texture(ao, TexCoords).r;

  gAlbedoRoughness.rgb = texture(albedo, TexCoords).rgb;
  gAlbedoRoughness.a = texture(roughness, TexCoords).r;

  vec3 N = texture(normal, TexCoords).rgb;
  N = N * 2.0 - 1.0;

  gNormalMetalic.rgb = normalize(TBN * N);
  gNormalMetalic.a = texture(metalic, TexCoords).r;

  gPosView = ViewPos;
  gNorView = normalize(TBN_View * N);

  gVelocity.xy = real_pos - last_pos;
}
