#include <iostream>

#include <glm/glm.hpp>

#include "simulake.hpp"

#include "renderer.hpp"
#include "shader.hpp"
#include "utils.hpp"

namespace simulake {

Renderer::Renderer(const std::uint32_t width, const std::uint32_t height,
                   const std::uint32_t cell_size) {
  /* set state variables */
  num_cells = 0;
  set_cell_size(cell_size);

  /* initialize opengl and shaders */
  initialize_graphics();
}

Renderer::~Renderer() {
  glDeleteProgram(shader.get_id());
  glUseProgram(0);

  glDeleteVertexArrays(1, &_VAO);
  glBindVertexArray(0);

  glDeleteBuffers(1, &_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteTextures(1, &_GRID_DATA_TEXTURE);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::initialize_graphics() noexcept {
  /* vertex and fragment shader locations */
  constexpr auto VERTEX_SHADER_PATH = "./shaders/vertex.glsl";
  constexpr auto FRAGMENT_SHADER_PATH = "./shaders/fragment.glsl";

  /* fullscreen quad vertices and tex coords */
  constexpr float FS_QUAD[] = {
      -1.0f, 1.0f,  0.0f, 1.0f, // top-left corner
      1.0f,  1.0f,  1.0f, 1.0f, // top-right corner
      -1.0f, -1.0f, 0.0f, 0.0f, // lower-left corner
      1.0f,  -1.0f, 1.0f, 0.0f, // lower-right corner
  };

  /* compile and bind shaders */
  shader = Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
  assert(shader.get_id() != 0);
  shader.use();

  /* create VAO, VBO, EBO, and texture */
  glGenVertexArrays(1, &_VAO);
  glGenBuffers(1, &_VBO);
  glGenBuffers(1, &_EBO);
  glGenTextures(1, &_GRID_DATA_TEXTURE);

  /* bind buffers */
  glBindVertexArray(_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _VBO);

  /* bind texture and set params */
  glBindTexture(GL_TEXTURE_2D, _GRID_DATA_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* location 0: vertex positions */
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(0));

  /* location 1: texture coordinates */
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  /* load vertices and tex coords into buffer */
  glBufferData(GL_ARRAY_BUFFER, sizeof(FS_QUAD), FS_QUAD, GL_STATIC_DRAW);
}

void Renderer::submit_grid(const Grid &grid) noexcept {
  const auto grid_width = grid.get_width();
  const auto grid_height = grid.get_height();
  const auto new_num_cells = grid_width * grid_height;

  /* update dimensions and regenerate grid */
  if (new_num_cells != num_cells) [[unlikely]] {
    num_cells = new_num_cells;
    grid_size[0] = grid_width;
    grid_size[1] = grid_height;
    viewport_size[0] = cell_size * grid_width;
    viewport_size[1] = cell_size * grid_height;
    glViewport(0, 0, viewport_size[0], viewport_size[1]);
  }

  update_grid_data_texture(grid);
}

void Renderer::submit_grid(DeviceGrid &grid) noexcept {
  const auto grid_width = grid.get_width();
  const auto grid_height = grid.get_height();
  const auto new_num_cells = grid_width * grid_height;

  /* update dimensions and regenerate grid */
  if (new_num_cells != num_cells) [[unlikely]] {
    num_cells = new_num_cells;
    grid_size[0] = grid_width;
    grid_size[1] = grid_height;
    viewport_size[0] = cell_size * grid_width;
    viewport_size[1] = cell_size * grid_height;
    glViewport(0, 0, viewport_size[0], viewport_size[1]);

    /* resize texture, fill with 0.f */
    std::vector<float> texture_data(num_cells * grid.get_stride(), 0.f);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, grid_width, grid_height, 0, GL_RG,
                 GL_FLOAT, texture_data.data());

    grid.set_texture_target(_GRID_DATA_TEXTURE);
  }
}

void Renderer::render() noexcept {
  /* clear framebuffer and draw */
  // glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::update_grid_data_texture(const Grid &grid) noexcept {
  /* make sure we are already resized to this grid size */
  assert(grid_size.x == grid.get_width());
  assert(grid_size.y == grid.get_height());

  const auto grid_width = grid_size.x;
  const auto grid_height = grid_size.y;

  /* number of cell attributes */
  const std::uint32_t stride = 2;

  std::vector<float> texture_data(num_cells * stride);
  for (std::uint32_t row = 0; row < grid_height; row += 1) {
    for (std::uint32_t col = 0; col < grid_width; col += 1) {
      const std::uint64_t base_index = (row * grid_width + col) * stride;

      // TODO(vir): add support for mass / other properties
      // clang-format off
      texture_data[base_index] = static_cast<float>(grid.type_at(grid_height - row - 1, col));
      texture_data[base_index + 1] = 1.f;
      // clang-format on
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, grid_width, grid_height, 0, GL_RG,
               GL_FLOAT, texture_data.data());
}

void Renderer::set_cell_size(const std::uint32_t new_size) noexcept {
  cell_size = new_size;
  shader.set_int("u_cell_size", cell_size);
}

} /* namespace simulake */
