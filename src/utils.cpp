#include <iostream>

#include "utils.hpp"

#include "cell.hpp"
#include "grid.hpp"

std::ostream &operator<<(std::ostream &stream, const simulake::CellType type) {
  switch (type) {
  case simulake::CellType::AIR:
    stream << '*';
    break;
  case simulake::CellType::WATER:
    stream << 'W';
    break;
  case simulake::CellType::OIL:
    stream << 'O';
    break;
  case simulake::CellType::SAND:
    stream << 'S';
    break;
  case simulake::CellType::FIRE:
    stream << 'F';
    break;
  case simulake::CellType::JELLO:
    stream << 'J';
    break;
  case simulake::CellType::SMOKE:
    stream << '#';
    break;

  default:
    stream << '-';
    break;
  }

  return stream;
}

std::ostream &operator<<(std::ostream &stream,
                         const simulake::BaseCell::context_t context) {
  // clang-format off
  stream << context.top_left << " " << context.top << " " << context.top_right << '\n';
  stream << context.left << " _ " << context.right << '\n';
  stream << context.bottom_left << " " << context.bottom << " " << context.bottom_right << '\n';
  // clang-format on
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const simulake::Grid &grid) {
  for (int i = 0; i < grid.get_height(); i += 1) {
    for (int j = 0; j < grid.get_width(); j += 1)
      stream << grid.type_at(i, j) << ' ';

    stream << '\n';
  }
  return stream;
}
