#pragma once

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

  /* simulate the next step */
  void simulate() noexcept;

  // const access
  CellType &at(std::uint32_t, std::uint32_t);

private:
  // all out of bounds cell point to here
  inline static const CellType OUT_OF_BOUNDS = CellType::NONE;

  // grid is represented as a 2D array of CellType (int ids)
  typedef std::vector<std::vector<CellType>> cell_grid_t;
  cell_grid_t _grid;

  uint32_t width, height;
};

} // namespace simulake
