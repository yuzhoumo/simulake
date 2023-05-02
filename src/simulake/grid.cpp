#include <cassert>
#include <iostream>
#include <optional>
#include <thread>

#include <omp.h>

#include "grid.hpp"
#include "utils.hpp"

namespace simulake {

Grid::Grid(const std::uint32_t _width, const std::uint32_t _height)
    : width(_width), height(_height) {
  stride = 2; // (type, mass)
  reset();

  const auto NUM_THREADS = std::thread::hardware_concurrency();
  omp_set_num_threads(std::max(1, static_cast<int>(NUM_THREADS - 2)));
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

void Grid::spawn_cells(const std::tuple<std::uint32_t, std::uint32_t> &center,
                       const std::uint32_t paint_radius,
                       const CellType paint_target) noexcept {

  float cx = std::get<0>(center);
  float cy = std::get<1>(center);

  const int y_start = std::max(static_cast<int>(cy - paint_radius), 0);
  const int y_end = std::min(static_cast<int>(cy + paint_radius),
                             static_cast<int>(height));

  const int x_start = std::max(static_cast<int>(cx - paint_radius), 0);
  const int x_end = std::min(static_cast<int>(cx + paint_radius),
                             static_cast<int>(width));

  for (int y = y_start; y < y_end; ++y) {
    for (int x = x_start; x < x_end; ++x) {
      int dist =
          static_cast<int>(sqrt(pow(static_cast<int>(cx) - x, 2) +
                                pow(static_cast<int>(cy) - y, 2)));

      bool should_paint =
          dist <= paint_radius and
          (paint_target == CellType::AIR or type_at(y, x) == CellType::AIR);

      if (should_paint) {
        set_state(y, x, paint_target);
        if (paint_target == CellType::WATER)
          _mass[y][x] = WaterCell::max_mass;
      }
    }
  }
}

void Grid::simulate() noexcept {
  // copy old grid into new
  _next_grid = _grid;
  _next_mass = _mass;

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
        FireCell::step({i, j}, *this);
        break;

      case CellType::JELLO:
        // JelloCell::step({i, j}, *this);
        break;

      case CellType::SMOKE:
        SmokeCell::step({i, j}, *this);
        break;

      case CellType::STONE:
        break;

      default: // CellType::NONE
        break;
      };
    }
  }

  // TODO(vir): add effects (smoke, fillins, etc)
  // second pass?

  // swap around
  std::swap(_grid, _next_grid);
  std::swap(_mass, _next_mass);
}

std::vector<float> Grid::serialize() const noexcept {
  std::vector<float> buf(width * height * stride);

  for (std::uint32_t row = 0; row < height; row += 1) {
    for (std::uint32_t col = 0; col < width; col += 1) {
      const std::uint64_t base_index = (row * width + col) * stride;

      buf[base_index] = static_cast<float>(type_at(height - row - 1, col));
      buf[base_index + 1] = static_cast<float>(mass_at(height - row - 1, col));
    }
  }

  return buf;
}

void Grid::deserialize(std::uint32_t new_width, std::uint32_t new_height,
                std::uint32_t new_stride, std::vector<float> buffer) noexcept {
  if (buffer.size() != new_width * new_height * new_stride) {
    std::cerr << "ERROR::GRID::DESERIALIZE: Incorrect buffer size" << std::endl;
    return;
  }

  if (width != new_width or height != new_height or stride != new_stride) {
    //TODO(joe): implment grid resize
    std::cerr << "NOT_IMPLEMENTED::GRID::DESERIALIZE: buffer resize" << std::endl;
    return;
  }

  for (std::uint32_t row = 0; row < height; row += 1) {
    for (std::uint32_t col = 0; col < width; col += 1) {
      const std::uint64_t base_index = (row * width + col) * stride;
      /* cast the buffer value back to CellType enum */
      CellType type = static_cast<CellType>(
            static_cast<std::uint32_t>(buffer[base_index]));
      float mass = buffer[base_index + 1];

      _grid[height - row - 1][col] = type;
      _mass[height - row - 1][col] = mass;
    }
  }
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
