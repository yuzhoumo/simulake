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

in vec2 tex_coord;
out vec4 frag_color;

void main() {
  vec4 grid_data = texture(u_grid_data_texture, tex_coord);

  int type = int(grid_data.r);
  float mass = grid_data.g;

  if (type == AIR_TYPE) {
    frag_color = vec4(0.5, 0.5, 0.5, 1.0); // gray
  } else if (type == SAND_TYPE) {
    frag_color = vec4(1.0, 1.0, 0.0, 1.0); // yellow but I change it to RED BUT IT'S NOT WORKINGGGG
  } else if (type == WATER_TYPE) {
    vec3 light_blue = vec3(0.0, 0.5, 1.0); // light blue
    vec3 dark_blue = vec3(0.0, 0.0, 0.5);  // dark blue
    frag_color = vec4(mix(light_blue, dark_blue, mass), 1.0); // interpolated
  } else if (type == STONE_TYPE) {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0); // black
  } else if (type == SMOKE_TYPE) {
    frag_color = vec4(0.0, 1.0, 0.0, 1.0); // green
  } else if (type == FIRE_TYPE) {
    frag_color = vec4(1.0, 0.0, 0.0, 1.0); // red
  } else {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0); // black
  }
}
