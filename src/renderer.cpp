#include <iostream>

#include <glad/glad.h>

#include "constants.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "utils.hpp"

namespace simulake {

Renderer::Renderer(const std::uint32_t _width, const std::uint32_t _height,
                   const std::uint32_t cell_size)
    : window(_width, _height, "simulake") {
  // set state variables
  set_cell_size(cell_size);
  set_viewport_size(_width, _height);

  // opengl and shaders initailize
  initialize_graphics();
}

Renderer::~Renderer() {
  glDeleteVertexArrays(1, &_VAO);
  glDeleteBuffers(1, &_VBO);
  glDeleteProgram(shader.get_id());
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

  // bind vertex data and load into buffer
  glBindVertexArray(_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);

  // configure vertex attributes
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
}

void Renderer::render(const Grid &grid) noexcept {
  glClear(GL_COLOR_BUFFER_BIT);

  // activate shader
  shader.use();

  // generate chunks
  const auto [chunks, chunk_indices] = generate_chunks(grid);

  // bind pipeline
  glBindVertexArray(_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);

  for (int i = 0; i < chunks.size(); ++i) {
    const auto &chunk = chunks[i];
    const auto &indices = chunk_indices[i];

    // set vertices
    glBufferData(GL_ARRAY_BUFFER, chunk.size() * sizeof(float), &chunk.front(),
                 GL_STREAM_DRAW);

    // set indices
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(std::uint32_t), &indices.front(),
                 GL_STREAM_DRAW);

    // draw triangles
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }

  window.swap_buffers();
}

std::tuple<Renderer::chunks_t, Renderer::chunk_indices_t>
Renderer::generate_chunks(const Grid &grid) noexcept {
  // PROFILE_FUNCTION();

  // make sure right size grid is passed for this renderer
  assert(grid.get_width() == viewport_size.x / cell_size);
  assert(grid.get_height() == viewport_size.y / cell_size);

  // TODO(joe): Change this value as needed
  const int chunk_size = 100;

  std::vector<std::vector<float>> chunks;
  std::vector<std::vector<unsigned int>> chunk_indices;

  const std::uint32_t n_chunks_x = viewport_size.x / chunk_size;
  const std::uint32_t n_chunks_y = viewport_size.y / chunk_size;
  chunks.reserve(n_chunks_x * n_chunks_y);

  // TODO(vir): parallelize loop, by changing push_back to indexed write
  for (int chunk_x = 0; chunk_x < viewport_size.x; chunk_x += chunk_size) {
    for (int chunk_y = 0; chunk_y < viewport_size.y; chunk_y += chunk_size) {
      const auto x_max = std::min(chunk_x + chunk_size, viewport_size.x);
      const auto y_max = std::min(chunk_y + chunk_size, viewport_size.y);

      std::vector<float> vertices;
      std::vector<std::uint32_t> indices;

      // reserve space to avoid resizing in loop
      vertices.reserve(16 * (x_max - chunk_x) * (y_max - chunk_y));
      indices.reserve(6 * (x_max - chunk_x) * (y_max - chunk_y));

      for (int i = chunk_x; i < x_max; ++i) {
        for (int j = chunk_y; j < y_max; ++j) {
          // TODO(joe): get mass from grid
          /* draw a quad in following order using triangle strip
           *
           * 2----3
           * |    |
           * |    |
           * 1----4
           */

          glm::vec2 bot_left{
              2.0f * (i * cell_size) / viewport_size.x - 1.0f,
              2.0f * (j * cell_size) / viewport_size.y - 1.0f,
          };

          glm::vec2 top_right{
              2.0f * (i + 1) * cell_size / viewport_size.x - 1.0f,
              2.0f * (j + 1) * cell_size / viewport_size.y - 1.0f,
          };

          // push vertices
          {
            const float cell_type = static_cast<float>(
                grid.type_at(static_cast<std::uint32_t>(i / cell_size),
                             static_cast<std::uint32_t>(j / cell_size)));

            // TODO(vir): get mass from grid
            const auto mass = 1.f;

            vertices.push_back(bot_left.x);
            vertices.push_back(bot_left.y);
            vertices.push_back(cell_type);
            vertices.push_back(mass);

            vertices.push_back(bot_left.x);
            vertices.push_back(top_right.y);
            vertices.push_back(cell_type);
            vertices.push_back(mass);

            vertices.push_back(top_right.x);
            vertices.push_back(bot_left.y);
            vertices.push_back(cell_type);
            vertices.push_back(mass);

            vertices.push_back(top_right.x);
            vertices.push_back(top_right.y);
            vertices.push_back(cell_type);
            vertices.push_back(mass);
          }

          // push indices
          {
            const std::uint32_t base_index = vertices.size() / 4;
            indices.push_back(base_index + 0);
            indices.push_back(base_index + 1);
            indices.push_back(base_index + 2);
            indices.push_back(base_index + 1);
            indices.push_back(base_index + 3);
            indices.push_back(base_index + 2);
          }
        }
      }

      // move into table
      chunks.emplace_back(std::move(vertices));
      chunk_indices.emplace_back(std::move(indices));
    }
  }

  return std::make_tuple(std::move(chunks), std::move(chunk_indices));
}

void Renderer::set_cell_size(const std::uint32_t new_size) noexcept {
  cell_size = new_size;
  shader.set_int("u_cell_size", cell_size);
}

void Renderer::set_viewport_size(const std::uint32_t width,
                                 const std::uint32_t height) noexcept {
  grid_size[0] = width / cell_size;
  grid_size[1] = height / cell_size;
  viewport_size[0] = width;
  viewport_size[1] = height;

  shader.set_float2("u_resolution", viewport_size);
  glViewport(0, 0, viewport_size[0], viewport_size[1]);
}

const Window &Renderer::get_window() const noexcept { return window; }

} // namespace simulake
