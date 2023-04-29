#include <iostream>
#include <utility>

#include "utils.hpp"

#include "cell.hpp"
#include "grid.hpp"

namespace simulake {

/* compute neighbor types and return as 8 element tuple */
[[nodiscard]] BaseCell::context_t
BaseCell::get_cell_context(const BaseCell::position_t &pos,
                           const Grid &grid) noexcept {
  const auto [x, y] = pos;
  return {
      grid.type_at(x - 1, y - 1), // top left
      grid.type_at(x - 1, y),     // top
      grid.type_at(x - 1, y + 1), // top right
      grid.type_at(x, y - 1),     // left
      grid.type_at(x, y + 1),     // right
      grid.type_at(x + 1, y - 1), // bottom left
      grid.type_at(x + 1, y),     // bottom
      grid.type_at(x + 1, y + 1)  // bottom right
  };
};

void AirCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  // will be flown into if
  // if (FALLS_DOWN(context.top)) {
  //   grid.set_at(x, y, context.top);
  // } else if (FALLS_DOWN(context.top_left)) {
  //   grid.set_at(x, y, context.top_left);
  // } else if (FALLS_DOWN(context.top_right)) {
  //   grid.set_at(x, y, context.top_right);
  // }
}

void SandCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  /* will flow down if possible */
  if (VACANT(context.bottom)) {
    grid.set_at(x, y, CellType::AIR);
    grid.set_at(x + 1, y, CellType::SAND);
  }

  /* else flow left if possible */
  else if (VACANT(context.bottom_left)) {
    grid.set_at(x, y, CellType::AIR);
    grid.set_at(x + 1, y - 1, CellType::SAND);
  }

  /* else flow right if possible */
  else if (VACANT(context.bottom_right)) {
    grid.set_at(x, y, CellType::AIR);
    grid.set_at(x + 1, y + 1, CellType::SAND);
  }
}

} /* namespace simulake */
