#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

#include "grid.hpp"
#include "utils.hpp"

namespace simulake {

Grid::Grid(const std::uint32_t _width, const std::uint32_t _height)
    : width(_width), height(_height) {
  stride = 2; // (type, mass)
  reset();
}

// reset empty grid (full of AIR cells)
void Grid::reset() noexcept {
  // construct in place
  _grid.resize(height, {width, CellType::AIR});

  // deep copy construct (same dimensions and contents)
  _next_grid = _grid;

  // Temp support for mass.
  _mass.resize(height, {});
  for (auto &row : _mass)
      row.resize(width, .0f);

  _next_mass = _mass;
}

Grid::~Grid() {
  // empty
}

void Grid::spawn_cells(const uint32_t x_center, const uint32_t y_center,
                       const uint32_t paint_radius,
                       const CellType paint_target) noexcept {

  const int y_start = std::max(static_cast<int>(y_center - paint_radius), 0);
  const int y_end = std::min(static_cast<int>(y_center + paint_radius),
                       static_cast<int>(height - 1));

  const int x_start = std::max(static_cast<int>(x_center - paint_radius), 0);
  const int x_end = std::min(static_cast<int>(x_center + paint_radius),
                       static_cast<int>(width - 1));

  for (int y = y_start; y < y_end; ++y) {
    for (int x = x_start; x < x_end; ++x) {
      uint32_t dist = static_cast<uint32_t>(
                    sqrt(pow(static_cast<int>(x_center) - x, 2) +
                         pow(static_cast<int>(y_center) - y, 2)));

      if (dist <= paint_radius and type_at(y, x) == CellType::AIR)
        set_state(y, x, paint_target);
    }
  }
}

void Grid::simulate() noexcept {
  // copy old grid into new
  _next_grid = _grid;
  _next_mass = _mass;

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
          WaterCell::step({i, j}, *this);
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

        case CellType::STONE:
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
  std::swap(_mass, _next_mass);
}

// reads current grid
[[nodiscard]] CellType Grid::type_at(std::uint32_t row,
                                     std::uint32_t col) const noexcept {
  if (row >= height || col >= width) [[unlikely]]
    return CellType::NONE;

  return _grid[row][col];
}

// renders next grid
bool Grid::set_at(std::uint32_t row, std::uint32_t col,
                  const CellType type) noexcept {
  // soft error when out of bounds
  if (row >= height || col >= width) [[unlikely]] {
    std::cerr << "ERROR::grid pos out of bounds: " << row << ' ' << col
              << std::endl;
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
  if (row >= height || col >= width) [[unlikely]] {
    std::cerr << "ERROR::grid pos out of bounds: " << row << ' ' << col
              << std::endl;
    return false;
  }

  else {
    _grid[row][col] = type;
    return true;
  }
}

// reads current mass grid
float Grid::mass_at(std::uint32_t row, std::uint32_t col) const noexcept {
    if (row >= height || col >= width)
        return 0;

    return _mass[row][col];
}

} // namespace simulake
