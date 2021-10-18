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

  mat3 normalMatrix = transpose(inverse(mat3(model)));
  vec3 T = normalize(normalMatrix * aTangent);
  vec3 N = normalize(normalMatrix * aNormal);
  T = normalize(T - dot(T, N) * N);
  vec3 B = cross(N, T);

  TBN = mat3(T, B, N);
}
