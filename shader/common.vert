#version 420 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 Normals;

void main() {
  gl_Position = projection * view * model * vec4(aPosition, 1.0);
  TexCoords = aTexCoords;
  Normals = mat3(transpose(inverse(model))) * aNormal;
}
