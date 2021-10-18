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

void main () {
  WorldPos = (model * vec4(aPos, 1.0f)).xyz;
  TexCoords = aTex;
  gl_Position = projection * view * vec4(WorldPos, 1.0);

  vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
  vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
  vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
  TBN = mat3(T, B, N);
}
