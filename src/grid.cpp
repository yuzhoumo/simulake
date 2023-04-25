#include <optional>
#include <vector>

#include "cell.hpp"
#include "grid.hpp"

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
}

Grid::~Grid() {
  // empty
}

void Grid::simulate() noexcept {
  // TODO(vir): parallelize loop
  for (std::uint32_t i = 0; i < height; i += 1) {
    for (std::uint32_t j = 0; j < width; j += 1) {
      switch (_grid[i][j]) {
      case CellType::AIR:
        // AirCell::step({i, j}, *this);
      case CellType::WATER:
        // WaterCell::step({i, j}, *this);
      case CellType::OIL:
        // OilCell::step({i, j}, *this);
      case CellType::SAND:
        // SandCell::step({i, j}, *this);
      case CellType::FIRE:
        // FireCell::step({i, j}, *this);
      case CellType::JELLO:
        // JelloCell::step({i, j}, *this);
      case CellType::SMOKE:
        // SmokeCell::step({i, j}, *this);

      default: // CellType::NONE
        break;
      };
    }
  }
}

CellType &Grid::at(std::uint32_t row, std::uint32_t col) {
  if (row >= height || col >= width)
    // return OUT_OF_BOUNDS;
    // NOTE(vir): figure out something better yet performant
    return const_cast<CellType &>(OUT_OF_BOUNDS);

  return _grid[row][col];
}

} // namespace simulake
