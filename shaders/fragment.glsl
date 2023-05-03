#version 330 core

#define AIR_TYPE 1
#define SMOKE_TYPE 2
#define FIRE_TYPE 3
#define WATER_TYPE 4
#define OIL_TYPE 5
#define SAND_TYPE 6
#define JELLO_TYPE 7
#define STONE_TYPE 8

uniform sampler2D u_grid_data_texture;

uniform vec2 u_resolution;
uniform vec2 u_grid_dim;
uniform vec2 u_mouse_pos;

uniform float u_spawn_radius;
uniform float u_cell_size;

in vec2 tex_coord;
out vec4 frag_color;

float rand(vec2 co) {
  // pseudo-random generator
  return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float fbm(vec2 st, int octaves) {
  // fractal brownian motion noise function
  float value = 0.0;
  float amplitude = 0.5;
  float frequency = 1.0;
  for (int i = 0; i < octaves; ++i) {
    value += amplitude * rand(st * frequency);
    amplitude *= 0.5;
    frequency *= 2.0;
  }
  return value;
}

vec4 shade_mouse_ring() {
  // normalize screen space coords and convert to grid coords
  vec2 st = gl_FragCoord.xy / u_resolution;
  vec2 grid_coords = st * u_grid_dim;

  // convert mouse position to grid coordinates (invert y for opengl coord)
  vec2 mouse_grid_pos =
    vec2(u_mouse_pos.x, u_resolution.y - u_mouse_pos.y) / u_resolution * u_grid_dim;

  // thickness of ring in grid cells
  float thickness = 1.0;
  float inner_radius = u_spawn_radius - thickness / 2.0;
  float outer_radius = u_spawn_radius + thickness / 2.0;

  float dist = length(grid_coords - mouse_grid_pos);
  float ring_alpha = step(inner_radius, dist) * (1.0 - step(outer_radius, dist));

  return vec4(1.0, 1.0, 1.0, ring_alpha);
}

vec4 shade_air() {
  return vec4(0.5, 0.5, 0.5, 1.0); // gray
}

vec4 shade_smoke(float mass) {
  float noise = fbm(gl_FragCoord.xy / u_resolution * u_grid_dim, 4);
  float gray_value = mix(0.5, 1.0, mass);
  vec3 gray_color = vec3(gray_value, gray_value, gray_value);
  return vec4(gray_color * (0.5 + 0.5 * noise), 1.0);
}

vec4 shade_fire() {
  vec2 st = gl_FragCoord.xy / u_resolution * u_grid_dim;
  float noise = fbm(st, 4);
  float mass = texture(u_grid_data_texture, tex_coord).g;

  // remap the mass value to a range of 0.0 to 1.0
  float normalized_mass = clamp(mass / 0.6, 0.0, 1.0);

  // create a gradient of colors from red to yellow based on mass and noise
  vec3 fire_color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), normalized_mass);
  fire_color *= 0.8 + 0.2 * noise;

  // make the fire appear more yellowish higher up in the flame
  fire_color *= mix(1.0, 1.4, smoothstep(0.0, 1.0, st.y / u_grid_dim.y));

  return vec4(fire_color, 1.0);
}

vec4 shade_water(float mass) {
  vec3 light_blue = vec3(0.0, 0.5, 1.0); // light blue
  vec3 dark_blue = vec3(0.0, 0.0, 0.5);  // dark blue
  return vec4(mix(light_blue, dark_blue, mass), 1.0); // interpolated
}

vec4 shade_oil() {
  // TODO: implement
  return vec4(0.0);
}

vec4 shade_sand() {
  float noise = fbm(gl_FragCoord.xy / u_resolution * u_grid_dim, 4);
  return vec4(vec3(0.9, 0.85, 0.4) * (0.5 + 0.5 * noise), 1.0);
}

vec4 shade_jello() {
  // TODO: implement
  return vec4(0.0);
}

vec4 shade_stone() {
  return vec4(0.0, 0.0, 0.0, 1.0); // black
}

vec4 shade_default() {
  return vec4(0.0, 0.0, 0.0, 1.0); // black
}

void main() {
  vec4 grid_data = texture(u_grid_data_texture, tex_coord);

  int cell_type = int(grid_data.r);
  float mass = grid_data.g;

  vec4 color = vec4(0.0);
  switch (cell_type) {
  case AIR_TYPE:
    color = shade_air();
    break;
  case SMOKE_TYPE:
    color = shade_smoke(mass);
    break;
  case FIRE_TYPE:
    color = shade_fire();
    break;
  case WATER_TYPE:
    color = shade_water(mass);
    break;
  case OIL_TYPE:
    color = shade_oil();
    break;
  case SAND_TYPE:
    color = shade_sand();
    break;
  case JELLO_TYPE:
    color = shade_jello();
    break;
  case STONE_TYPE:
    color = shade_stone();
    break;
  default:
    color = shade_default();
  }

  vec4 mouse_ring = shade_mouse_ring();
  frag_color = mix(color, mouse_ring, mouse_ring.a);
}
