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

uniform ivec2 u_resolution;
uniform vec2 u_mouse_pos;

uniform int u_spawn_radius;
uniform int u_cell_size;

in vec2 tex_coord;
out vec4 frag_color;

vec4 shade_mouse_ring() {
  vec2 st = gl_FragCoord.xy / vec2(u_resolution);
  vec2 mouse_pos = vec2(u_mouse_pos.x, u_resolution.y - u_mouse_pos.y) /
                                                           vec2(u_resolution);
  float rad = float(u_spawn_radius * u_cell_size);
  float thickness = 1.5 * float(u_cell_size);
  vec2 aspect_ratio = vec2(1.0, u_resolution.x / u_resolution.y);
  vec2 diff = (st - mouse_pos) * aspect_ratio;
  float dist = length(diff * vec2(u_resolution));

  float inner_radius = rad - thickness / 2.0;
  float outer_radius = rad + thickness / 2.0;
  float ring_alpha = step(inner_radius, dist) *
                    (1.0 - step(outer_radius, dist));

  return vec4(1.0, 1.0, 1.0, ring_alpha);
}

vec4 shade_air() {
  return vec4(0.5, 0.5, 1.0, 1.0); // gray
}

vec4 shade_smoke() {
  return vec4(0.0, 1.0, 0.0, 1.0); // green
}

vec4 shade_fire() {
  return vec4(1.0, 0.0, 0.0, 1.0); // red
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
  return vec4(1.0, 1.0, 0.0, 1.0); // yellow
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
    color = shade_smoke();
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
