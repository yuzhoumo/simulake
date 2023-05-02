#include <iostream>

#include "simulake.hpp"

#include "renderer.hpp"
#include "utils.hpp"

namespace simulake {

Renderer::Renderer(const std::uint32_t width, const std::uint32_t height,
                   const std::uint32_t cell_size) {
  /* set state variables */
  this->cell_size = cell_size;
  num_cells = 0;

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

void Renderer::set_viewport_size(const std::uint32_t width,
                                 const std::uint32_t height) const noexcept {
  glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void Renderer::submit_shader_uniforms(
    const uniform_opts_t &uniform_updates) const noexcept {
  for (const auto &[uniform, value] : uniform_updates) {
    switch (uniform) {
    case Renderer::UniformId::CELL_SIZE:
      shader.set_int("u_cell_size", std::get<int>(value));
      break;
    case Renderer::UniformId::SPAWN_RADIUS:
      shader.set_int("u_spawn_radius", std::get<int>(value));
      break;
    case Renderer::UniformId::MOUSE_POS:
      shader.set_float2("u_mouse_pos", std::get<glm::vec2>(value));
      break;
    case Renderer::UniformId::RESOLUTION:
      shader.set_int2("u_resolution", std::get<glm::ivec2>(value));
      break;
    }
  }
}

void Renderer::submit_grid(GridBase *grid) noexcept {
  const auto grid_width = grid->get_width();
  const auto grid_height = grid->get_height();
  const auto new_num_cells = grid_width * grid_height;
  const auto is_device_grid = grid->is_device_grid();

  /* update dimensions and regenerate grid */
  if (new_num_cells != num_cells) [[unlikely]] {
    num_cells = new_num_cells;
    grid_size[0] = grid_width;
    grid_size[1] = grid_height;

    if (is_device_grid) {
      /* resize texture, fill with 0.f */
      std::vector<DeviceGrid::device_cell_t> texture_data(num_cells);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, grid_width, grid_height, 0,
                   GL_RG, GL_FLOAT, texture_data.data());

      static_cast<DeviceGrid *>(grid)->set_texture_target(_GRID_DATA_TEXTURE);
    }
  }

  /* update cpu grid texture */
  if (!is_device_grid) {
    const auto texture_data = grid->serialize();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, grid_width, grid_height, 0, GL_RG,
                 GL_FLOAT, texture_data.data());
  }
}

void Renderer::render() const noexcept {
  /* clear framebuffer and draw */
  // glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} /* namespace simulake */
