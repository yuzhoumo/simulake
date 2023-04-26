#version 330 core

layout (location = 0) in vec4 i_cell_data;

out vec4 color;

#define AIR_TYPE 1
#define WATER_TYPE 4
#define SAND_TYPE 6

void main() {
  int type = int(i_cell_data.z);
  float mass = i_cell_data.w;

  if (type == AIR_TYPE) {
    color = vec4(0.5, 0.5, 0.5, 1.0);
  } else if (type == SAND_TYPE) {
    color = vec4(1.0, 1.0, 0.0, 1.0);
  } else if (type == WATER_TYPE) {
    color = vec4(0.0, 0.0, 1.0, 1.0);
  } else {
    color = vec4(0.0, 0.5, 0.0, 1.0);
  }

  gl_Position = vec4(i_cell_data.xy, 0.0, 1.0);
}
