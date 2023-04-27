#version 330 core

#define AIR_TYPE 1
#define WATER_TYPE 4
#define SAND_TYPE 6
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
    frag_color = vec4(1.0, 1.0, 0.0, 1.0); // yellow
  } else if (type == WATER_TYPE) {
    frag_color = vec4(0.0, 0.0, 1.0, 1.0); // blue
  } else if (type == STONE_TYPE) {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0); // black
  } else {
    frag_color = vec4(0.0, 0.0, 0.0, 1.0); // black
  }
}
