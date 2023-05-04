#ifndef SIMULAKE_GRID_HPP
#define SIMULAKE_GRID_HPP

#include <optional>
#include <random>
#include <vector>

#include "cell.hpp"
#include "grid_base.hpp"

namespace simulake {

/* Grid is a cpu implementation of CA grid */

class Grid : public GridBase {
public:
  /* default: initialize empty grid */
  explicit Grid(const std::uint32_t, const std::uint32_t);
  ~Grid() = default;

  /* enable moves */
  explicit Grid(Grid &&) = default;
  Grid &operator=(Grid &&) = default;

  /* disable copies */
  explicit Grid(const Grid &) = delete;
  Grid &operator=(const Grid &) = delete;

  /* iterate the simulation by one step */
  void simulate(float) noexcept override;

  /* reset the grid to air cells (empty) */
  void reset() noexcept override;

  /* spawn cells, given grid coords, radius in cells, and cell type */
  void spawn_cells(const std::tuple<std::uint32_t, std::uint32_t> &,
                   const std::uint32_t, const CellType) noexcept override;

  /* is not device grid */
  constexpr bool is_device_grid() const noexcept override { return false; }

  /* accessor methods for grid dimensions */
  inline std::uint32_t get_width() const noexcept override { return width; }
  inline std::uint32_t get_height() const noexcept override { return height; }

  /* accessor method for stride between serialized buffer items */
  inline std::uint32_t get_stride() const noexcept override { return stride; }

  /* saves grid to float buffer */
  serialized_grid_t serialize() const noexcept override;

  /* loads grid from float buffer */
  void deserialize(const serialized_grid_t &) noexcept override;

  /* get cell type at given position */
  cell_data_t cell_at(std::uint32_t, std::uint32_t) const noexcept;

  /* set cell type at given position. returns true of successful */
  bool set_next(std::uint32_t, std::uint32_t, const cell_data_t) noexcept;
  bool set_curr(std::uint32_t, std::uint32_t, const cell_data_t) noexcept;

  inline float get_delta_time() { return delta_time; }

  /* check if x, y is inside the grid */
  inline bool in_bounds(std::uint32_t x, std::uint32_t y) {
    return x >= 0 and y >= 0 and x < width and y < height;
  }

  /* check if x, y is inside the grid */
  inline bool is_empty(std::uint32_t x, std::uint32_t y) {
    return in_bounds(x, y) and _grid[y][x].type == CellType::AIR;
  }

  /* check if any of 8 neighboring cells are liquid, return
   * ivec3 with 1 in first pos denoting if liquid was found, and
   * the x, y coord of the liquid or all zeros if not. */
  inline glm::ivec3 is_in_liquid(std::uint32_t x, std::uint32_t y) {
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        int ny = y + dy;
        int nx = x + dx;

        // Skip the original cell
        if (dx == 0 && dy == 0 || !in_bounds(nx, ny)) {
          continue;
        }

        CellType type = cell_at(nx, ny).type;

        if (BaseCell::is_liquid(type)) {
          return glm::ivec3(1, nx, ny);
        }
      }
    }

    return glm::ivec3(0, 0, 0);
  }

private:
  /* grid is represented as a 2D array of cell_data_t */
  typedef std::vector<std::vector<cell_data_t>> grid_data_t;

  /* buffers */
  grid_data_t _grid;      // completed last grid
  grid_data_t _next_grid; // next grid being computed, swap at end of simulate

  std::uint32_t width;
  std::uint32_t height;
  std::uint32_t stride;

  float delta_time = 0.0f;
};

} /* namespace simulake */

#endif
