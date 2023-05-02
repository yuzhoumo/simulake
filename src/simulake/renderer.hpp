#ifndef SIMULAKE_RENDERER_HPP
#define SIMULAKE_RENDERER_HPP

#include <glm/glm.hpp>
#include <unordered_map>
#include <variant>

#include "device_grid.hpp"
#include "grid.hpp"
#include "shader.hpp"

namespace simulake {

class Renderer {
public:
  /* create and initialize renderer */
  explicit Renderer(const std::uint32_t width = 800,
                    const std::uint32_t height = 600,
                    const std::uint32_t cell_size = 4);

  /* disable moves */
  explicit Renderer(Renderer &&) = delete;
  Renderer &operator=(Renderer &&) = delete;

  /* disable copies */
  explicit Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  /* free up resources on deletion */
  ~Renderer();

  /* submit new grid data to renderer */
  void submit_grid(const Grid &) noexcept;
  void submit_grid(DeviceGrid &) noexcept;

  enum class UniformId { CELL_SIZE, SPAWN_RADIUS, MOUSE_POS, RESOLUTION };
  typedef std::variant<glm::ivec2, glm::vec2, int> shader_uniform_t;
  typedef std::unordered_map<UniformId, shader_uniform_t> uniform_opts_t;

  /* submit updated uniforms to shader, use enum + unordered mapping to
   * update only the specified shader uniforms */
  void submit_shader_uniforms(const uniform_opts_t &) const noexcept;

  /* render frame based on dataptr */
  void render() const noexcept;

  /* set the viewport dimensions */
  void set_viewport_size(const std::uint32_t,
                         const std::uint32_t) const noexcept;

private:
  /* update grid texture based on new simulation state */
  void update_grid_data_texture(const Grid &) const noexcept;

  /* initialize opengl and shaders */
  void initialize_graphics() noexcept;

  glm::ivec2 grid_size;    /* grid width, height in cells */
  std::uint32_t num_cells; /* number of cells to render */
  std::uint32_t cell_size; /* each cell pixels = (cell_size * cell_size) */

  Shader shader;
  GLuint _VAO, _VBO, _GRID_DATA_TEXTURE;
};

} // namespace simulake

#endif
