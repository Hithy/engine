#version 420 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out mat3 TBN;

void main() {
  Normal = mat3(transpose(inverse(model))) * aNormal;
  TexCoords = aTexCoords;
  FragPos = vec3(model * vec4(aPosition, 1.0f));
  gl_Position = projection * view * model * vec4(aPosition, 1.0);

  vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
  vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
  vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
  TBN = mat3(T, B, N);
}
