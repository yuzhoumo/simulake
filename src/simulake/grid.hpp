#ifndef SIMULAKE_GRID_HPP
#define SIMULAKE_GRID_HPP

#include <optional>
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
  void simulate() noexcept override;

  /* reset the grid to air cells (empty) */
  void reset() noexcept override;

  /* spawn cells, given grid coords, radius in cells, and cell type */
  void spawn_cells(const std::tuple<std::uint32_t, std::uint32_t> &,
                   const std::uint32_t,
                   const CellType) noexcept override;

  /* is not device grid */
  constexpr bool is_device_grid() const noexcept override {
    return false;
  }

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

private:

  /* grid is represented as a 2D array of cell_data_t */
  typedef std::vector<std::vector<cell_data_t>> grid_data_t;

  /* buffers */
  grid_data_t _grid;      // completed last grid
  grid_data_t _next_grid; // next grid being computed, swap at end of simulate

  std::uint32_t width;
  std::uint32_t height;
  std::uint32_t stride;
};

} /* namespace simulake */

#endif
