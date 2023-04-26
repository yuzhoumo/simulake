#include <iostream>

#include <glad/glad.h>

#include "constants.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "utils.hpp"

namespace simulake {

Renderer::Renderer(const std::uint32_t width, const std::uint32_t height,
                   const std::uint32_t cell_size)
    : window(width, height, "simulake") {
  // set state variables
  buffer_resized = true;
  num_cells = (width / cell_size) * (height / cell_size);
  set_cell_size(cell_size);

  // initialize opengl and shaders
  initialize_graphics();
}

Renderer::~Renderer() {
  glDeleteVertexArrays(1, &_VAO);
  glDeleteBuffers(1, &_VBO);
  glDeleteProgram(shader.get_id());
  // TODO(vir): check if grid_data_texture is deleted
}

void Renderer::initialize_graphics() noexcept {
  /* vertex and fragment shader locations */
  constexpr auto VERTEX_SHADER_PATH = "./shaders/vertex.glsl";
  constexpr auto FRAGMENT_SHADER_PATH = "./shaders/fragment.glsl";

  // compile shaders
  shader = Shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
  assert(shader.get_id() != 0);

  // create VAO, VBO, EBO
  glGenVertexArrays(1, &_VAO);
  glGenBuffers(1, &_VBO);
  glGenBuffers(1, &_EBO);
  glGenTextures(1, &_GRID_DATA_TEXTURE);

  // bind vertex data and load into buffer
  glBindVertexArray(_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);

  // configure vertex attributes
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
}

void Renderer::submit_grid(const Grid &grid) noexcept {
  const auto grid_width = grid.get_width();
  const auto grid_height = grid.get_width();

  const auto curr_num_cells = grid_width * grid_height;

  // update dimensions and regenerate grid
  if (curr_num_cells != num_cells) [[unlikely]] {
    num_cells = curr_num_cells;
    grid_size[0] = grid_width;
    grid_size[1] = grid_height;
    viewport_size[0] = cell_size * grid_width;
    viewport_size[1] = cell_size * grid_height;
    glViewport(0, 0, viewport_size[0], viewport_size[1]);

    regenerate_grid();
    buffer_resized = true;
  }

  update_grid_data_texture(grid);
}

void Renderer::render() noexcept {
  glClear(GL_COLOR_BUFFER_BIT);

  // activate shader
  shader.use();

  // activate cells texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _GRID_DATA_TEXTURE);

  // bind vertex buffer and draw triangles
  glBindVertexArray(_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);

  if (buffer_resized) [[unlikely]] {
    // bind vertices
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
                 &(vertices.front()), GL_STREAM_DRAW);

    // bind indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(std::uint32_t) * ebo_indices.size(),
                 &(ebo_indices.front()), GL_STREAM_DRAW);
  } else {
    buffer_resized = false;

    // bind vertices
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size(),
                    vertices.data());

    // bind indices
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    sizeof(std::uint32_t) * ebo_indices.size(),
                    ebo_indices.data());
  }

  // draw triangles
  glDrawElements(GL_TRIANGLES, ebo_indices.size(), GL_UNSIGNED_INT, 0);

  // refresh window
  window.swap_buffers();
}

void Renderer::regenerate_grid() noexcept {
  vertices.clear();
  ebo_indices.clear();

  vertices.reserve(16 * grid_size.x * grid_size.y);
  ebo_indices.reserve(6 * grid_size.x * grid_size.y);

  for (int i = 0; i < grid_size.x; ++i) {
    for (int j = 0; j < grid_size.y; ++j) {
      // push vertices
      {
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

      // push indices
      {
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

void Renderer::update_grid_data_texture(const Grid &grid) noexcept {
  glBindTexture(GL_TEXTURE_2D, _GRID_DATA_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  const auto grid_width = grid.get_width();
  const auto grid_height = grid.get_height();

  std::vector<float> texture_data(num_cells * 2);
  for (size_t i = 0; i < num_cells; ++i) {
    const auto row = grid_height - (i / grid_width) - 1;
    const auto col = i % grid_width;

    // texture_data[i * 2] = static_cast<float>(grid_data.cells[i].type);
    // texture_data[i * 2 + 1] = grid_data.cells[i].mass;

    // TODO(vir): add support for mass and other properties
    texture_data[i * 2] = static_cast<float>(grid.type_at(row, col));
    texture_data[i * 2 + 1] = 1.f;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, grid_width, grid_height, 0, GL_RG,
               GL_FLOAT, texture_data.data());
}

void Renderer::set_cell_size(const std::uint32_t new_size) noexcept {
  cell_size = new_size;
  shader.set_int("u_cell_size", cell_size);
}

const Window &Renderer::get_window() const noexcept { return window; }

} // namespace simulake
