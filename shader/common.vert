#version 420 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

void main() {
  Normal = mat3(transpose(inverse(model))) * aNormal;
  TexCoords = aTexCoords;
  FragPos = vec3(model * vec4(aPosition, 1.0f));
  gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
