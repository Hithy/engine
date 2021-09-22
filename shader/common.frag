#version 420 core

struct PointLight {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;

  int enable_shadow;
  samplerCube shadow_map;
};
#define NR_POINT_LIGHTS_MAX 5

struct DirLight {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  int enable_shadow;
  mat4 shadow_vp;
  sampler2D shadow_map;
};
#define NR_DIR_LIGHTS_MAX 5

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

uniform vec3 viewPos;

uniform int texture_diffuse_enable;
uniform sampler2D texture_diffuse1;
uniform int texture_specular_enable;
uniform sampler2D texture_specular1;
uniform int texture_normal_enable;
uniform sampler2D texture_normal1;

uniform PointLight point_light_list[NR_POINT_LIGHTS_MAX];
uniform DirLight direction_light_list[NR_DIR_LIGHTS_MAX];

uniform int point_light_count;
uniform int direction_light_count;

float CalcDirShadow(DirLight light, vec3 normal)
{
  float shadow_ratio = 0.0f;
  if (light.enable_shadow > 0) {
    vec4 shadow_tex = light.shadow_vp * vec4(FragPos, 1.0f);
    vec3 projCoords = shadow_tex.xyz / shadow_tex.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec2 texelSize = 1.0 / textureSize(light.shadow_map, 0);
    float bias = max(0.05 * (1.0 - dot(normal, normalize(-light.direction))), 0.005);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(light.shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow_ratio += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow_ratio /= 9.0;
  }

  return shadow_ratio;
}

float CalcPointShadow(PointLight light) {
  float shadow_ratio = 0.0f;
  if (light.enable_shadow > 0) {
    vec3 fragToLight = FragPos - light.position;
    float closestDepth = texture(light.shadow_map, normalize(fragToLight)).r * 125.0f;
    float currentDepth = length(fragToLight);

    float bias = 0.05;
    shadow_ratio = currentDepth - bias > closestDepth ? 1.0 : 0.0;
  }
  return shadow_ratio;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);

  float diff = max(dot(normal, lightDir), 0.0);

  vec3 midDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(midDir, normal), 0.0), 32);

  float shadow_ratio = CalcDirShadow(light, normal);

  vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
  vec3 diffuse = light.diffuse * vec3(texture(texture_diffuse1, TexCoords)) * diff * (1.0f - shadow_ratio);
  vec3 specular = light.specular * vec3(texture(texture_specular1, TexCoords)) * spec * (1.0f - shadow_ratio);
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

  float shadow_ratio = CalcPointShadow(light);

  vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords)) * (1.0f - shadow_ratio);
  vec3 diffuse = light.diffuse * vec3(texture(texture_diffuse1, TexCoords)) * diff * (1.0f - shadow_ratio);
  vec3 specular = light.specular * vec3(texture(texture_specular1, TexCoords)) * spec * (1.0f - shadow_ratio);

  return (ambient + diffuse + specular) * attenuation;
}

void main() {
  vec3 result = vec3(0.0f, 0.0f, 0.0f);
  vec3 normal = normalize(Normal);
  if (texture_normal_enable > 0) {
    normal = texture(texture_normal1, TexCoords).rgb;
    normal = normal * 2.0 - 1.0;   
    normal = normalize(TBN * normal);
  }
  
  vec3 viewDir = normalize(viewPos - FragPos);

  for (int i = 0; i < point_light_count; i++) {
    result += CalcPointLight(point_light_list[i], normal, FragPos, viewDir);
  }

  for (int i = 0; i < direction_light_count; i++) {
    result += CalcDirLight(direction_light_list[i], normal, viewDir);
  }

  FragColor = vec4(result, 1.0f);
}
