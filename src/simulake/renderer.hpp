#ifndef SIMULAKE_RENDERER_HPP
#define SIMULAKE_RENDERER_HPP

#include <vector>

#include <glm/glm.hpp>

#include "device_grid.hpp"
#include "grid.hpp"
#include "shader.hpp"

namespace simulake {

// template <typename G>
// concept grid_t = requires(G grid) {
//                    { grid.reset() };
//                    { grid.simulate() };
//                    { grid.get_width() } -> std::integral;
//                    { grid.get_height() } -> std::integral;
//                    { grid.get_height() } -> std::integral;
//                  };

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

  /* render frame based on dataptr */
  void render() noexcept;

private:
  /* update grid texture based on new simulation state */
  void update_grid_data_texture(const Grid &) noexcept;

  /* initialize opengl and shaders */
  void initialize_graphics() noexcept;

  glm::ivec2 grid_size;     /* grid width, height in cells */
  glm::ivec2 viewport_size; /* viewport width, height in pixels */
  std::uint32_t num_cells;  /* number of cells to render */
  std::uint32_t cell_size;  /* each cell pixels = (cell_size * cell_size) */

  Shader shader;
  GLuint _VAO, _VBO, _GRID_DATA_TEXTURE;
};

} // namespace simulake

#endif
