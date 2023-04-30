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

in vec2 tex_coord;
out vec4 frag_color;

void shade_air() {
  frag_color = vec4(0.5, 0.5, 0.5, 1.0); // gray
}

void shade_smoke() {
  frag_color = vec4(0.0, 1.0, 0.0, 1.0); // green
}

void shade_fire() {
  frag_color = vec4(1.0, 0.0, 0.0, 1.0); // red
}

void shade_water(float mass) {
  vec3 light_blue = vec3(0.0, 0.5, 1.0); // light blue
  vec3 dark_blue = vec3(0.0, 0.0, 0.5);  // dark blue
  frag_color = vec4(mix(light_blue, dark_blue, mass), 1.0); // interpolated
}

void shade_oil() {
  // TODO: implement
}

void shade_sand() {
  frag_color = vec4(1.0, 1.0, 0.0, 1.0); // yellow
}

void shade_jello() {
  // TODO: implement
}

void shade_stone() {
  frag_color = vec4(0.0, 0.0, 0.0, 1.0); // black
}

void shade_default() {
  frag_color = vec4(0.0, 0.0, 0.0, 1.0); // black
}

void main() {
  vec4 grid_data = texture(u_grid_data_texture, tex_coord);

  int cell_type = int(grid_data.r);
  float mass = grid_data.g;

  switch (cell_type) {
  case AIR_TYPE:
    shade_air();
    break;
  case SMOKE_TYPE:
    shade_smoke();
    break;
  case FIRE_TYPE:
    shade_fire();
    break;
  case WATER_TYPE:
    shade_water(mass);
    break;
  case OIL_TYPE:
    shade_oil();
    break;
  case SAND_TYPE:
    shade_sand();
    break;
  case JELLO_TYPE:
    shade_jello();
    break;
  case STONE_TYPE:
    shade_stone();
    break;
  default:
    shade_default();
  }
}
