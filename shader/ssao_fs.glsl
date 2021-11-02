#version 330 core

// gbuffer
uniform sampler2D gViewPos;
uniform sampler2D gViewNor;

const int kernelSize = 32;
uniform vec3 samples[kernelSize];
// 4x4
uniform sampler2D texture_noise;
const vec2 noiseScale = vec2(1920.0/4.0, 1080.0/4.0);

uniform mat4 projection;
uniform vec2 jitter;

in vec2 TexCoords;
out float ao_result;

void main() {
  vec3 view_pos = texture(gViewPos, TexCoords).xyz;
  vec3 view_nor = texture(gViewNor, TexCoords).xyz;
  vec3 noise_vec = texture(texture_noise, TexCoords * noiseScale).xyz;

  mat4 jitter_projection = projection;
  jitter_projection[3][0] = 2.0 * jitter.x * -view_pos.z;
  jitter_projection[3][1] = 2.0 * jitter.y * -view_pos.z;

  vec3 tangent   = normalize(noise_vec - view_nor * dot(noise_vec, view_nor));
  vec3 bitangent = cross(view_nor, tangent);
  mat3 TBN       = mat3(tangent, bitangent, view_nor);

  float occlusion = 0.0;
  float radius = 1.0;
  float bias = 0.025;

  for(int i = 0; i < kernelSize; ++i)
  {
    // get sample position
    vec3 sampleViewPos = TBN * samples[i]; // from tangent to view-space
    sampleViewPos = view_pos + sampleViewPos * radius;
    vec4 sampleProjPos = jitter_projection * vec4(sampleViewPos, 1.0);

    vec3 offset = sampleProjPos.xyz / sampleProjPos.w;
    offset = offset * 0.5 + 0.5; // transform to range 0.0 - 1.0  
    float screenDepth = texture(gViewPos, offset.xy).z;

    float rangeCheck = smoothstep(0.0, 1.0, radius / abs(view_pos.z - screenDepth));
    occlusion += (screenDepth >= sampleViewPos.z + bias ? 1.0 : 0.0) * rangeCheck;
  }

  occlusion = 1.0 - (occlusion / kernelSize);
  ao_result = occlusion;
}
