#include <iostream>
#include <utility>
#include <random>

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
}

void AirCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);
}

void StoneCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);
}

void SandCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  /* will flow down if possible */
  if (IS_FLUID(context.bottom)) {
    grid.set_at(x, y, CellType::AIR);
    grid.set_at(x + 1, y, CellType::SAND);
  }

  /* else flow left if possible */
  else if (IS_FLUID(context.bottom_left)) {
    grid.set_at(x, y, CellType::AIR);
    grid.set_at(x + 1, y - 1, CellType::SAND);
  }

  /* else flow right if possible */
  else if (IS_FLUID(context.bottom_right)) {
    grid.set_at(x, y, CellType::AIR);
    grid.set_at(x + 1, y + 1, CellType::SAND);
  }
}

void FireCell::helper(CellType curr, Grid &grid, int x, int y) {
  if (IS_FLAMMABLE(curr)) {
    grid.set_at(x, y, CellType::FIRE);
  } else if (curr == CellType::AIR) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    float p = 0.4;
    double rand_num = dis(gen);
    if (rand_num < p) {
      grid.set_at(x, y, CellType::SMOKE);
    }
  }
}
void FireCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  FireCell::helper(context.top, grid, x - 1, y);
  FireCell::helper(context.top_left, grid, x - 1, y - 1);
  FireCell::helper(context.top_right, grid, x - 1, y + 1);

  FireCell::helper(context.bottom, grid, x + 1, y);
  FireCell::helper(context.bottom_left, grid, x + 1, y - 1);
  FireCell::helper(context.bottom_right, grid, x + 1, y + 1);

  FireCell::helper(context.left, grid, x, y - 1);
  FireCell::helper(context.right, grid, x, y + 1);


  // With a low probability, the fire will die down and become air
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0, 1);
  float p = 0.01;
  double rand_num = dis(gen);
  if (rand_num < p) {
    grid.set_at(x, y, CellType::AIR);
  }
}

void SmokeCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  // Initialize random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0, 1);
  float p = 0.4;

  // Generate a random number between 0 and 1
  double rand_num = dis(gen);

  /* will flow up if possible */
  if (IS_FLUID(context.top)) {
    if (rand_num < p) {
        grid.set_at(x, y, CellType::AIR);
        grid.set_at(x - 1, y, CellType::SMOKE);
    }
  }

  /* else flow left if possible */
  else if (IS_FLUID(context.top_left)) {
    if (rand_num < p) {
      grid.set_at(x, y, CellType::AIR);
      grid.set_at(x - 1, y - 1, CellType::SMOKE);
    }
  }

  /* else flow right if possible */
  else if (IS_FLUID(context.top_right)) {
    if (rand_num < p) {
      grid.set_at(x, y, CellType::AIR);
      grid.set_at(x - 1, y + 1, CellType::SMOKE);
    }
  }
}

void WaterCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  float flow;
  float remaining_mass = grid.mass_at(x, y);
  if (remaining_mass <= 0) {
    grid.set_at(x, y, CellType::AIR);
    return;
  }

  // Flow to block below.
  if (IS_FLUID(context.bottom)) {
    // Convert to water.
    grid.set_at(x + 1, y, CellType::WATER);

    flow = get_stable_state_b(remaining_mass + grid.mass_at(x + 1, y))
        - grid.mass_at(x + 1, y);

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, std::min(max_speed, remaining_mass));
    grid._next_mass[x][y] -= flow;
    grid._next_mass[x + 1][y] += flow;
    remaining_mass -= flow;
  }

  if (remaining_mass <= 0) {
    grid.set_at(x, y, CellType::AIR);
    return;
  }

  // Equalize water with right block.
  if (IS_FLUID(context.right)) {
    // Convert to water.
    grid.set_at(x, y + 1, CellType::WATER);

    flow = (grid.mass_at(x, y) - grid.mass_at(x, y + 1)) / 4;

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, remaining_mass);
    grid._next_mass[x][y] -= flow;
    grid._next_mass[x][y + 1] += flow;
    remaining_mass -= flow;
  }

  if (remaining_mass <= 0) {
    grid.set_at(x, y, CellType::AIR);
    return;
  }

  // Equalize water with left block.
  if (IS_FLUID(context.left)) {
    // Convert to water.
    grid.set_at(x, y - 1, CellType::WATER);

    flow = (grid.mass_at(x, y) - grid.mass_at(x, y - 1)) / 4;

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, remaining_mass);
    grid._next_mass[x][y] -= flow;
    grid._next_mass[x][y - 1] += flow;
    remaining_mass -= flow;
  }

  if (remaining_mass <= 0) {
    grid.set_at(x, y, CellType::AIR);
    return;
  }

  // If compressed, flow up.
  if (IS_FLUID(context.top)) {
    // Convert to water.
    grid.set_at(x, y, CellType::WATER);

    flow = remaining_mass
        - get_stable_state_b(remaining_mass + grid.mass_at(x - 1, y));

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, std::min(max_speed, remaining_mass));

    grid._next_mass[x][y] -= flow;
    grid._next_mass[x - 1][y] += flow;
    remaining_mass -= flow;
  }
}

} /* namespace simulake */
