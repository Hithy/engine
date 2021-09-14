#version 420 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normals;

// uniform float Elapse;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main() {
    FragColor = texture(texture_diffuse1, TexCoords);
}
