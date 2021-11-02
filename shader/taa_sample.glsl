#version 430 core

uniform sampler2D last_frame;
uniform sampler2D jitter_frame;
uniform sampler2D velocity;

uniform uint screen_width;
uniform uint screen_height;

uniform float blend_ratio;

in vec2 TexCoords;
out vec4 FragColor;

vec3 rgb2ycogo(vec3 color) {
  float Co = color.r - color.b;
  float t = color.b + (Co / 2);
  float Cg = color.g - t;
  float Y = t + (Cg / 2);

  return vec3(Y, Co, Cg);
}

vec3 ycogo2rgb(vec3 color) {
  float Y = color.r;
  float Co = color.g;
  float Cg = color.b;

  float t = Y - (Cg / 2);
  float G = Cg + t;
  float B = t - (Co / 2);
  float R = B + Co;

  return vec3(R, G, B);
}

vec3 ClipAABB(vec3 aabbMin, vec3 aabbMax, vec3 prevSample, vec3 avg)
{
	vec3 r = prevSample - avg;
	vec3 rmax = aabbMax - avg.xyz;
	vec3 rmin = aabbMin - avg.xyz;

	float eps = 0.000001f;

	if (r.x > rmax.x + eps)
		r *= (rmax.x / r.x);
	if (r.y > rmax.y + eps)
		r *= (rmax.y / r.y);
	if (r.z > rmax.z + eps)
		r *= (rmax.z / r.z);

	if (r.x < rmin.x - eps)
		r *= (rmin.x / r.x);
	if (r.y < rmin.y - eps)
		r *= (rmin.y / r.y);
	if (r.z < rmin.z - eps)
		r *= (rmin.z / r.z);

	return avg + r;
}

void main() {
  vec2 screen_pos = gl_FragCoord.xy / vec2(screen_width, screen_height);
  vec3 new_color = texture(jitter_frame, screen_pos).xyz;
  vec2 offset = texture(velocity, screen_pos).xy;
  vec3 old_color = texture(last_frame, screen_pos - offset).xyz;
  vec3 clip_color = old_color;

  if (gl_FragCoord.x > 0 && gl_FragCoord.x < 1920 && gl_FragCoord.y > 0 && gl_FragCoord.y < 1080) {
    vec3 l_color = texture(jitter_frame, (gl_FragCoord.xy + vec2(-1.0, 0.0)) / vec2(screen_width, screen_height)).xyz;
    vec3 r_color = texture(jitter_frame, (gl_FragCoord.xy + vec2(1.0, 0.0)) / vec2(screen_width, screen_height)).xyz;
    vec3 u_color = texture(jitter_frame, (gl_FragCoord.xy + vec2(0.0, 1.0)) / vec2(screen_width, screen_height)).xyz;
    vec3 d_color = texture(jitter_frame, (gl_FragCoord.xy + vec2(0.0, -1.0)) / vec2(screen_width, screen_height)).xyz;

    l_color = rgb2ycogo(l_color);
    r_color = rgb2ycogo(r_color);
    u_color = rgb2ycogo(u_color);
    d_color = rgb2ycogo(d_color);

    float min_x = min(min(l_color.x, r_color.x), min(u_color.x, d_color.x));
    float min_y = min(min(l_color.y, r_color.y), min(u_color.y, d_color.y));
    float min_z = min(min(l_color.z, r_color.z), min(u_color.z, d_color.z));

    float max_x = max(max(l_color.x, r_color.x), max(u_color.x, d_color.x));
    float max_y = max(max(l_color.y, r_color.y), max(u_color.y, d_color.y));
    float max_z = max(max(l_color.z, r_color.z), max(u_color.z, d_color.z));

    vec3 aabb_min = vec3(min_x, min_y, min_z);
    vec3 aabb_max = vec3(max_x, max_y, max_z);

    clip_color = ycogo2rgb(ClipAABB(aabb_min, aabb_max, rgb2ycogo(old_color), (aabb_max + aabb_min) / 2.0));
  }
  
  vec3 blend_color = blend_ratio * clip_color + (1 - blend_ratio) * new_color;
  FragColor = vec4(blend_color, 1.0);
}
