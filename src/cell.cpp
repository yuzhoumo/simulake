#include <utility>

#include "cell.hpp"
#include "grid.hpp"

namespace simulake {

// calculate neighbour types and return them as tuple of 8 elements
[[no_discard]] BaseCell::context_t
get_cell_context(const BaseCell::position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  return {
      grid.at(x - 1, y - 1), // top left
      grid.at(x - 1, y),     // top
      grid.at(x - 1, y + 1), // top right
      grid.at(x, y - 1),     // left
      grid.at(x, y + 1),     // right
      grid.at(x + 1, y - 1), // bottom left
      grid.at(x + 1, y),     // bottom
      grid.at(x + 1, y + 1)  // bottom right
  };
};

} // namespace simulake
