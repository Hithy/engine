#version 330 core

layout (location = 0) out vec4 gPosAO;
layout (location = 1) out vec4 gAlbedoRoughness;
layout (location = 2) out vec4 gNormalMetalic;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D metalic;
uniform sampler2D roughness;
uniform sampler2D ao;

uniform mat4 model;

in vec3 WorldPos;
in vec2 TexCoords;
in mat3 TBN;

void main() {
  gPosAO.rgb = WorldPos;
  gPosAO.a = texture(ao, TexCoords).r;

  gAlbedoRoughness.rgb = texture(albedo, TexCoords).rgb;
  gAlbedoRoughness.a = texture(roughness, TexCoords).r;
  // gAlbedoRoughness.a = texture(roughness, TexCoords).r;

  vec3 N = texture(normal, TexCoords).rgb;
  N = N * 2.0 - 1.0;
  N = normalize(TBN * N); 

  gNormalMetalic.rgb = N;
  gNormalMetalic.a = texture(metalic, TexCoords).r;
}