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

void main () {
  WorldPos = (model * vec4(aPos, 1.0f)).xyz;
  ViewPos = (view * model * vec4(aPos, 1.0f)).xyz;
  TexCoords = aTex;
  gl_Position = projection * view * vec4(WorldPos, 1.0);

  vec3 T = normalize(aTangent);
  vec3 B = normalize(aBitangent);
  vec3 N = normalize(aNormal);
  TBN = mat3(transpose(inverse(model))) * mat3(T, B, N);
  TBN_View = mat3(transpose(inverse(view))) * TBN;
}
