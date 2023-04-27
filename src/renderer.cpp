#include <iostream>

#include <glm/glm.hpp>

#include "simulake.hpp"

#include "constants.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "utils.hpp"

namespace simulake {

Renderer::Renderer(const std::uint32_t width, const std::uint32_t height,
                   const std::uint32_t cell_size)
    : window(width, height, "simulake") {

  /* set state variables */
  num_cells = 0;
  set_cell_size(cell_size);

  /* initialize opengl and shaders */
  initialize_graphics();
}

Renderer::~Renderer() {
  /* delete gl objects */
  glDeleteProgram(shader.get_id());
  glDeleteVertexArrays(1, &_VAO);
  glDeleteBuffers(1, &_VBO);
  glDeleteTextures(1, &_GRID_DATA_TEXTURE);

  /* unbind gl objects */
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::initialize_graphics() noexcept {
  /* vertex and fragment shader locations */
  constexpr auto VERTEX_SHADER_PATH = "./shaders/vertex.glsl";
  constexpr auto FRAGMENT_SHADER_PATH = "./shaders/fragment.glsl";

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
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);

  /* bind texture and set params */
  glBindTexture(GL_TEXTURE_2D, _GRID_DATA_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* location 0: vertex positions */
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        static_cast<void *>(0));

  /* location 1: texture coordinates */
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
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

    regenerate_grid();
    regenerate_pipeline();
  }

  update_grid_data_texture(grid);
}

void Renderer::render() noexcept {
  /* clear framebuffer colors */
  glClear(GL_COLOR_BUFFER_BIT);

  /* draw triangles */
  glDrawElements(GL_TRIANGLES, ebo_indices.size(), GL_UNSIGNED_INT, 0);

  /* flush framebuffer to window */
  window.swap_buffers();
}

void Renderer::regenerate_grid() noexcept {
  vertices.clear();
  ebo_indices.clear();

  vertices.reserve(16 * grid_size.x * grid_size.y);
  ebo_indices.reserve(6 * grid_size.x * grid_size.y);

  for (int i = 0; i < grid_size.x; ++i) {
    for (int j = 0; j < grid_size.y; ++j) {
      /*
       * 1----3  generate a quad in screen-space
       * |    |  vertices 1, 2 are shared using EBO:
       * |    |    triangle1: (0, 1, 2)
       * 0----2    triangle2: (1, 3, 2)
       */

      { /* push vertices and texture coordinates */
        const glm::vec2 bot_left{
            2.0f * (i * cell_size) / viewport_size.x - 1.0f,
            2.0f * (j * cell_size) / viewport_size.y - 1.0f,
        };

        const glm::vec2 top_right{
            2.0f * (i + 1) * cell_size / viewport_size.x - 1.0f,
            2.0f * (j + 1) * cell_size / viewport_size.y - 1.0f,
        };

        const glm::vec2 tex_bot_left{
            static_cast<float>(i) / grid_size.x,
            static_cast<float>(j) / grid_size.y,
        };

        const glm::vec2 tex_top_right{
            static_cast<float>(i + 1) / grid_size.x,
            static_cast<float>(j + 1) / grid_size.y,
        };

        vertices.push_back(bot_left.x);
        vertices.push_back(bot_left.y);
        vertices.push_back(tex_bot_left.x);
        vertices.push_back(tex_bot_left.y);

        vertices.push_back(bot_left.x);
        vertices.push_back(top_right.y);
        vertices.push_back(tex_bot_left.x);
        vertices.push_back(tex_top_right.y);

        vertices.push_back(top_right.x);
        vertices.push_back(bot_left.y);
        vertices.push_back(tex_top_right.x);
        vertices.push_back(tex_bot_left.y);

        vertices.push_back(top_right.x);
        vertices.push_back(top_right.y);
        vertices.push_back(tex_top_right.x);
        vertices.push_back(tex_top_right.y);
      }

      { /* push EBO indices */
        const std::uint32_t base_index = vertices.size() / 4;

        ebo_indices.push_back(base_index + 0);
        ebo_indices.push_back(base_index + 1);
        ebo_indices.push_back(base_index + 2);

        ebo_indices.push_back(base_index + 1);
        ebo_indices.push_back(base_index + 3);
        ebo_indices.push_back(base_index + 2);
      }
    }
  }
}

void Renderer::regenerate_pipeline() noexcept {
  /* NOTE(vir): dont need to bind resources (see initialize_graphics)
   * only need to update data on device */

  /* push vertex data */
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
               &(vertices.front()), GL_STREAM_DRAW);

  /* push index data */
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(std::uint32_t) * ebo_indices.size(),
               &(ebo_indices.front()), GL_STREAM_DRAW);
}

void Renderer::update_grid_data_texture(const Grid &grid) noexcept {
  /* make sure we are already resized to this grid size */
  assert(grid_size.x == grid.get_width());
  assert(grid_size.y == grid.get_height());

  const auto grid_width = grid_size.x;
  const auto grid_height = grid_size.y;

  /* number of cell attributes */
  const uint32_t stride = 2;

  std::vector<float> texture_data(num_cells * stride);
  for (std::uint32_t row = 0; row < grid_height; row += 1) {
    for (std::uint32_t col = 0; col < grid_width; col += 1) {
      const std::uint64_t base_index = (row * grid_width + col) * stride;

      // clang-format off
      // TODO(vir): add support for mass / other properties
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

const Window &Renderer::get_window() const noexcept { return window; }

} /* namespace simulake */
