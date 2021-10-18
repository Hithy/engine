#version 330 core

const float PI = 3.14159265359;

in vec3 LocalPos;
uniform samplerCube skybox;

out vec4 FragColor;

vec3 getIrradiance(vec3 N);

void main() {
  FragColor = vec4(getIrradiance(normalize(LocalPos)), 1.0);
}

vec3 getIrradiance(vec3 N) {
  vec3 out_color = vec3(0.0);

  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = normalize(cross(up, N));
  up = normalize(cross(N, right));

  float delta = 0.025;
  int count = 0;
  for (float phi = 0.0; phi < PI * 2.0; phi += delta) {
    for (float theta = 0.0; theta < PI * 0.5; theta += delta) {
      float x = sin(theta) * cos(phi);
      float y = sin(theta) * sin(phi);
      float z = cos(theta);

      count++;
      vec3 sample_vec = right * x + up * y + N * z;
      out_color += texture(skybox, sample_vec).rgb * cos(theta) * sin(theta);
    }
  }
  out_color = PI * out_color * (1.0 / float(count));

  return out_color;
}