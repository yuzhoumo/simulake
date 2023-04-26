#include <iostream>
#include <glad/glad.h>

#include "include/shader.hpp"
#include "include/renderer.hpp"
#include "include/constants.hpp"

Renderer::Renderer(GridData grid_data) :
                   _shader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH) {

  /* check if shaders compiled */
  if (0 == _shader.get_id())
    std::cerr << "ERROR::SHADER_CREATION_FAILURE" << std::endl;

  /* set state variables */
  _set_cell_size(4);
  _set_viewport_size(_cell_size * grid_data.width, _cell_size * grid_data.height);

  /* create gl objects */
  glGenVertexArrays(1, &_VAO);
  glGenBuffers(1, &_VBO);
  glGenBuffers(1, &_IBO);

  /* bind vertex data and load into buffer */
  glBindVertexArray(_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _VBO);

  /* configure vertex attribute pointers */
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
}

void Renderer::render(GridData grid_data, GLFWwindow*& window) {
  glClear(GL_COLOR_BUFFER_BIT);

  _shader.use();
  _set_grid_size(grid_data);
  _set_viewport_size(_cell_size * _grid_size);

  std::vector<std::vector<float>> chunks = _generate_chunks(grid_data);

  glBindVertexArray(_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, _VBO);

  for (const auto& chunk : chunks) {
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * chunk.size(),
                 &(chunk.front()), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, chunk.size());
  }

  glfwSwapBuffers(window);
}

std::vector<std::vector<float>> Renderer::_generate_chunks(GridData grid_data) {
  const int chunk_size = 512; // FIXME: Change this value as needed
  std::vector<std::vector<float>> chunks;

  for (int chunk_x = 0; chunk_x < _grid_size.x; chunk_x += chunk_size) {
    for (int chunk_y = 0; chunk_y < _grid_size.y; chunk_y += chunk_size) {
      std::vector<float> vertices;

      for (int i = chunk_x; i < std::min(chunk_x + chunk_size, _grid_size.x); ++i) {
        for (int j = chunk_y; j < std::min(chunk_y + chunk_size, _grid_size.y); ++j) {
          /* draw a quad in following order
           * [square, not to scale]
           *
           * 2----3/4
           * |    / |
           * |   /  |
           * |  /   |
           * | /    |
           * 1/5----6
           */

          glm::vec2 bot_left{
            2.0f * (i * _cell_size) / _viewport_size.x - 1.0f,
            2.0f * (j * _cell_size) / _viewport_size.y - 1.0f
          };

          glm::vec2 top_right{
            2.0f * (i + 1) * _cell_size / _viewport_size.x - 1.0f,
            2.0f * (j + 1) * _cell_size / _viewport_size.y - 1.0f
          };

          int index = j * grid_data.width + i;
          int cell_type = static_cast<float>(grid_data.cells[index].type);
          float mass = grid_data.cells[index].mass;

          vertices.emplace_back(bot_left.x);
          vertices.emplace_back(bot_left.y);
          vertices.emplace_back(cell_type);
          vertices.emplace_back(mass);

          vertices.emplace_back(bot_left.x);
          vertices.emplace_back(top_right.y);
          vertices.emplace_back(cell_type);
          vertices.emplace_back(mass);

          vertices.emplace_back(top_right.x);
          vertices.emplace_back(top_right.y);
          vertices.emplace_back(cell_type);
          vertices.emplace_back(mass);

          vertices.emplace_back(top_right.x);
          vertices.emplace_back(top_right.y);
          vertices.emplace_back(cell_type);
          vertices.emplace_back(mass);

          vertices.emplace_back(bot_left.x);
          vertices.emplace_back(bot_left.y);
          vertices.emplace_back(cell_type);
          vertices.emplace_back(mass);

          vertices.emplace_back(top_right.x);
          vertices.emplace_back(bot_left.y);
          vertices.emplace_back(cell_type);
          vertices.emplace_back(mass);
        }
      }

      chunks.push_back(vertices);
    }
  }

  return chunks;
}

void Renderer::_set_cell_size(int cell_size) {
  _cell_size = cell_size;
  _shader.set_int("u_cell_size", cell_size);
}

void Renderer::_set_grid_size(GridData grid_data) {
  _num_cells = grid_data.width * grid_data.height;
  _grid_size[0] = grid_data.width;
  _grid_size[1] = grid_data.height;
}

void Renderer::_set_viewport_size(int width, int height) {
  _viewport_size[0] = width;
  _viewport_size[1] = height;
  _shader.set_float2("u_resolution", _viewport_size);
  glViewport(0, 0, width, height);
}

void Renderer::_set_viewport_size(glm::ivec2 size) {
  _set_viewport_size(size[0], size[1]);
}
