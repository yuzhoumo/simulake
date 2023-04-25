#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

#include "grid.hpp"
#include "utils.hpp"

namespace simulake {

Grid::Grid(const std::uint32_t _width, const std::uint32_t _height)
    : width(_width), height(_height) {
  reset();
}

// reset empty grid (full of AIR cells)
void Grid::reset() noexcept {
  _grid.resize(height, {});
  for (auto &row : _grid)
    row.resize(width, CellType::AIR);

  // deep copy construct (same dimensions and contents)
  _next_grid = _grid;
}

Grid::~Grid() {
  // empty
}

void Grid::simulate() noexcept {
  // copy old grid into new
  _next_grid = _grid;

  // TODO(vir): parallelize loop
  {
#pragma omp parallel for
#if 1
    for (int i = height - 1; i >= 0; i -= 1) {
      for (int j = width - 1; j >= 0; j -= 1) {

#else
    for (int i = 0; i < height; i += 1) {
      for (int j = 0; j < width; j += 1) {
#endif
        switch (_grid[i][j]) {
        case CellType::AIR:
          AirCell::step({i, j}, *this);
          break;

        case CellType::WATER:
          // WaterCell::step({i, j}, *this);
          break;

        case CellType::OIL:
          // OilCell::step({i, j}, *this);
          break;

        case CellType::SAND:
          SandCell::step({i, j}, *this);
          break;

        case CellType::FIRE:
          // FireCell::step({i, j}, *this);
          break;

        case CellType::JELLO:
          // JelloCell::step({i, j}, *this);
          break;

        case CellType::SMOKE:
          // SmokeCell::step({i, j}, *this);
          break;

        default: // CellType::NONE
          break;
        };
      }
    }
  }

  // TODO(vir): add effects (smoke, fillins, etc)
  // second pass?

  // swap around
  std::swap(_grid, _next_grid);
}

// reads current grid
CellType Grid::type_at(std::uint32_t row, std::uint32_t col) const noexcept {
  if (row >= height || col >= width)
    return CellType::NONE;

  return _grid[row][col];
}

// renders next grid
bool Grid::set_at(std::uint32_t row, std::uint32_t col,
                  const CellType type) noexcept {
  // soft error when out of bounds
  if (row >= height && col >= width) {
    std::cerr << "out of bounds: " << row << ' ' << col << std::endl;
    return false;
  }

  else {
    _next_grid[row][col] = type;
    return true;
  }
}

// set current grid
bool Grid::set_state(std::uint32_t row, std::uint32_t col,
                     const CellType type) noexcept {
  // soft error when out of bounds
  if (row >= height && col >= width) {
    std::cerr << "out of bounds: " << row << ' ' << col << std::endl;
    return false;
  }

  else {
    _grid[row][col] = type;
    return true;
  }
}

} // namespace simulake
