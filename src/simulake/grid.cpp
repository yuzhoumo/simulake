#include <cassert>
#include <iostream>
#include <optional>
#include <thread>
#include <random>

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

void Grid::reset() noexcept {
  // construct in place
  _grid.resize(height, {width, { CellType::AIR, 0.0f, false } });

  // deep copy construct (same dimensions and contents)
  _next_grid = _grid;
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
          (paint_target == CellType::AIR or cell_at(y, x).type == CellType::AIR);

      cell_data_t cell = {
        .type = paint_target, .mass = 0.0f, .updated = true };

      if (should_paint) {
        if (paint_target == CellType::WATER) {
          cell.mass = WaterCell::max_mass;
          set_curr(y, x, cell);
        } else if (paint_target == CellType::FIRE) {
          std::mt19937 gen(std::random_device{}());
          std::uniform_real_distribution<float> dis(0.6f, 1.0f);
          cell.mass = dis(gen);
          set_curr(y, x, cell);
          //TODO(joe): save generator in a common location
        } else {
          set_curr(y, x, cell);
        }
      }
    }
  }
}

void Grid::simulate() noexcept {
  // copy old grid into new
  _next_grid = _grid;

#pragma omp parallel for
#if 1
  for (int i = height - 1; i >= 0; i -= 1) {
    for (int j = width - 1; j >= 0; j -= 1) {

#else
  for (int i = 0; i < height; i += 1) {
    for (int j = 0; j < width; j += 1) {
#endif
      switch (_grid[i][j].type) {
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
}

GridBase::serialized_grid_t Grid::serialize() const noexcept {
  std::vector<float> buf(width * height * stride);

  for (std::uint32_t row = 0; row < height; row += 1) {
    for (std::uint32_t col = 0; col < width; col += 1) {
      const std::uint64_t base_index = (row * width + col) * stride;
      const cell_data_t cell = cell_at(height - row - 1, col);

      buf[base_index] = static_cast<float>(cell.type);
      buf[base_index + 1] = static_cast<float>(cell.mass);
    }
  }

  return { .width = width, .height = height, .stride = stride, .buffer = buf };
}

void Grid::deserialize(const GridBase::serialized_grid_t &data) noexcept {
  if (width != data.width or height != data.height or stride != data.stride) {
    //TODO(joe): implement grid resize
    std::cerr << "NOT_IMPLEMENTED::GRID::DESERIALIZE: buffer resize" << std::endl;
    return;
  }

  for (std::uint32_t row = 0; row < height; row += 1) {
    for (std::uint32_t col = 0; col < width; col += 1) {
      const std::uint64_t base_index = (row * width + col) * stride;
      /* cast the buffer value back to CellType enum */
      CellType type = static_cast<CellType>(
            static_cast<std::uint32_t>(data.buffer[base_index]));
      float mass = data.buffer[base_index + 1];

      _grid[height - row - 1][col] = {
          .type = type, .mass = mass, .updated = false };
    }
  }
}

cell_data_t Grid::cell_at(std::uint32_t row, std::uint32_t col) const noexcept {
  if (row >= height || col >= width) [[unlikely]]
    return cell_data_t{};

  return _grid[row][col];
}

bool Grid::set_next(std::uint32_t row, std::uint32_t col,
                    const cell_data_t cell) noexcept {
  if (row >= height || col >= width) [[unlikely]] {
    std::cerr << "ERROR::GRID: out of bound: " << row << ' ' << col
        << std::endl;
    return false;
  } else {
    _next_grid[row][col] = cell;
    return true;
  }
}

bool Grid::set_curr(std::uint32_t row, std::uint32_t col,
                    const cell_data_t cell) noexcept {
  if (row >= height || col >= width) [[unlikely]] {
    std::cerr << "ERROR::GRID: out of bound: " << row << ' ' << col
        << std::endl;
    return false;
  } else {
    _grid[row][col] = cell;
    return true;
  }
}

} // namespace simulake
