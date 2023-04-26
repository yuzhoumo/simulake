#ifndef SIMULAKE_RENDERER_HPP
#define SIMULAKE_RENDERER_HPP

#include <vector>

#include <glm/glm.hpp>

#include "grid.hpp"
#include "shader.hpp"
#include "window.hpp"

namespace simulake {
class Window;

class Renderer {
public:
  typedef std::vector<std::vector<float>> chunks_t;
  typedef std::vector<std::vector<unsigned int>> chunk_indices_t;

  /* initialize renderer */
  explicit Renderer(const std::uint32_t = 800, const std::uint32_t = 600,
                    const std::uint32_t = 4);

  // disable moves
  explicit Renderer(Renderer &&) = delete;
  Renderer &operator=(Renderer &&) = delete;

  // disable copies
  explicit Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  /* free up resources on deletion */
  ~Renderer();

  /* render frame based on dataptr */
  void render(const Grid &) noexcept;

  /* get a reference to renderer window */
  const Window &get_window() const noexcept;

private:
  // generate chunks based on simulake::Grid
  std::tuple<chunks_t, chunk_indices_t> generate_chunks(const Grid &) noexcept;

  /* initialize opengl and shaders */
  void initialize_graphics() noexcept;

  /* set state */
  void set_cell_size(const std::uint32_t) noexcept;
  void set_viewport_size(const std::uint32_t, const std::uint32_t) noexcept;

  glm::ivec2 grid_size;     /* grid width, height in cells */
  glm::ivec2 viewport_size; /* viewport width, height in pixels */
  std::uint32_t num_cells;  /* number of cells to render */
  std::uint32_t cell_size;  /* each cell pixels = (cell_size * cell_size) */

  Window window;
  Shader shader;
  GLuint _VAO, _VBO, _EBO;
};

} // namespace simulake

#endif
