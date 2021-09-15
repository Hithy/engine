#version 420 core

struct PointLight {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};
#define NR_POINT_LIGHTS_MAX 10

struct DirLight {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
#define NR_DIR_LIGHTS_MAX 10

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform PointLight point_light_list[NR_POINT_LIGHTS_MAX];
uniform DirLight direction_light_list[NR_DIR_LIGHTS_MAX];

uniform int point_light_count;
uniform int direction_light_count;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);

  float diff = max(dot(normal, lightDir), 0.0);

  vec3 midDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(midDir, normal), 0.0), 32);

  vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
  vec3 diffuse = light.diffuse * vec3(texture(texture_diffuse1, TexCoords)) * diff;
  vec3 specular = light.specular * vec3(texture(texture_specular1, TexCoords)) * spec;
  return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  vec3 lightDir = normalize(light.position - fragPos);
  float diff = max(dot(normal, lightDir), 0.0);

  vec3 midDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(midDir, normal), 0.0), 32);

  vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
  vec3 diffuse = light.diffuse * vec3(texture(texture_diffuse1, TexCoords)) * diff;
  vec3 specular = light.specular * vec3(texture(texture_specular1, TexCoords)) * spec;

  return (ambient + diffuse + specular) * attenuation;
}

void main() {
  vec3 result = vec3(0.0f, 0.0f, 0.0f);
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  for (int i = 0; i < point_light_count; i++) {
    result += CalcPointLight(point_light_list[i], norm, FragPos, viewDir);
  }

  for (int i = 0; i < direction_light_count; i++) {
    result += CalcDirLight(direction_light_list[i], norm, viewDir);
  }

  FragColor = vec4(result, 1.0f);
}
