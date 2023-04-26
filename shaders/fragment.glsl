#version 330 core

uniform vec2 u_resolution;
uniform int u_cell_size;

in vec4 color;
out vec4 frag_color;

void main() {
  frag_color = color;
}
