#ifndef SIMULAKE_GRID_HPP
#define SIMULAKE_GRID_HPP

#include <optional>
#include <vector>

#include "cell.hpp"

namespace simulake {
/* represents the CA-Grid */
class Grid {
public:
  /* initialize empty grid */
  explicit Grid(const std::uint32_t,
                const std::uint32_t); // default constructor

  // enable moves
  explicit Grid(Grid &&) = default;   // move constructor
  Grid &operator=(Grid &&) = default; // move assignment operator

  // disable copies
  explicit Grid(const Grid &) = delete;   // copy constructor
  Grid &operator=(const Grid &) = delete; // copy assignment operator

  /* dealloc memory on destruction */
  ~Grid();

  /* reset to empty grid */
  void reset() noexcept;

  /* simulate the next step, update grid */
  void simulate() noexcept;

  /* get cell type at given position */
  CellType type_at(std::uint32_t, std::uint32_t) const noexcept;

  /* set cell type at given position. returns true of successful */
  bool set_at(std::uint32_t, std::uint32_t, const CellType) noexcept;
  bool set_state(std::uint32_t, std::uint32_t, const CellType) noexcept;

  /* utils */
  // void set_random_state(const std::uint32_t) noexcept;

  // getters
  inline std::uint32_t get_width() const noexcept { return width; }
  inline std::uint32_t get_height() const noexcept { return height; }

private:
  // grid is represented as a 2D array of CellType (int ids)
  typedef std::vector<std::vector<CellType>> cell_grid_t;

  // buffers
  cell_grid_t _grid;      // completed last grid
  cell_grid_t _next_grid; // next grid being computed, swap at end of simulate

  std::uint32_t width, height;
};

} // namespace simulake

#endif
