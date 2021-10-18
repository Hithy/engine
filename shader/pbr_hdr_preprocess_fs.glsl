#version 330 core

const float PI = 3.14159265359;

in vec3 LocalPos;

out vec4 FragColor;

uniform sampler2D hdrTexture;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 getCubeUV(vec3 v) {
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

void main() {
  vec2 uv = getCubeUV(normalize(LocalPos));
  FragColor = vec4(texture(hdrTexture, uv).rgb, 1.0);
}
