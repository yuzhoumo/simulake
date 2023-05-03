#include <cassert>
#include <iostream>
#include <optional>
#include <thread>
#include <random>

#include <omp.h>

#include "grid.hpp"
#include "simulake/cell.hpp"
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
  /* construct in place */
  _grid.resize(height, {width, cell_data_t{ .type = CellType::AIR } });

  /* deep copy construct (same dimensions and contents) */
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
          (paint_target == CellType::AIR or cell_at(x, y).type == CellType::AIR);

      cell_data_t cell = {
        .type = paint_target,
        .mass = 0.0f,
        .velocity = glm::vec2{0.0f},
        .updated = false
      };

      if (should_paint) {
        if (paint_target == CellType::WATER) {
          cell.mass = WaterCell::max_mass;
          set_curr(x, y, cell);
        } else if (paint_target == CellType::FIRE) {
          std::mt19937 gen(std::random_device{}());
          std::uniform_real_distribution<float> dis(0.6f, 1.0f);
          cell.mass = dis(gen);
          set_curr(x, y, cell);
          //TODO(joe): save generator in a common location
        } else {
          set_curr(x, y, cell);
        }
      }
    }
  }
}

void Grid::simulate(float delta_time) noexcept {
  this->delta_time = delta_time;
  _next_grid = _grid;

  for (int x = 0; x < width; x += 1) {
    for (int y = 0; y < height; y += 1) {
      switch (_grid[y][x].type) {
      case CellType::AIR:
        AirCell::step({x, y}, *this);
        break;

      case CellType::WATER:
        WaterCell::step({x, y}, *this);
        break;

      case CellType::OIL:
        // OilCell::step({x, y}, *this);
        break;

      case CellType::SAND:
        SandCell::step({x, y}, *this);
        break;

      case CellType::FIRE:
        FireCell::step({x, y}, *this);
        break;

      case CellType::JELLO:
        // JelloCell::step({x, y}, *this);
        break;

      case CellType::SMOKE:
        SmokeCell::step({x, y}, *this);
        break;

      case CellType::STONE:
        //StoneCell::step({x, y}, *this);
        break;

      default: // CellType::NONE
        break;
      };
    }
  }

  for (int x = 0; x < width; x += 1) {
    for (int y = 0; y < height; y += 1) {
      _grid[y][x].updated = false;
    }
  }

  // TODO(vir): add effects (smoke, fillins, etc)
  // second pass?

  // swap around
  std::swap(_grid, _next_grid);
}

GridBase::serialized_grid_t Grid::serialize() const noexcept {
  std::vector<float> buf(width * height * stride);

  for (std::uint32_t y = 0; y < height; y += 1) {
    for (std::uint32_t x = 0; x < width; x += 1) {
      const std::uint64_t base_index = (y * width + x) * stride;
      const cell_data_t cell = cell_at(x, height - y - 1);

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
        .type = type,
        .mass = mass,
        .velocity = glm::vec2{0.0f},
        .updated = false
      };
    }
  }
}

cell_data_t Grid::cell_at(std::uint32_t x, std::uint32_t y) const noexcept {
  if (y >= height || x >= width) [[unlikely]]
    return cell_data_t{};

  return _grid[y][x];
}

bool Grid::set_next(std::uint32_t x, std::uint32_t y,
                    const cell_data_t cell) noexcept {
  if (y >= height || x >= width) [[unlikely]] {
    std::cerr << "ERROR::GRID: out of bound: " << x << ' ' << y
        << std::endl;
    return false;
  } else {
    _next_grid[y][x] = cell;
    return true;
  }
}

bool Grid::set_curr(std::uint32_t x, std::uint32_t y,
                    const cell_data_t cell) noexcept {
  if (y >= height || x >= width) [[unlikely]] {
    std::cerr << "ERROR::GRID: out of bound: " << x << ' ' << y
        << std::endl;
    return false;
  } else {
    _grid[y][x] = cell;
    return true;
  }
}

} // namespace simulake
