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
    grid.cell_at(x - 1, y - 1).type, // top left
    grid.cell_at(x - 1, y).type,     // top
    grid.cell_at(x - 1, y + 1).type, // top right
    grid.cell_at(x, y - 1).type,     // left
    grid.cell_at(x, y + 1).type,     // right
    grid.cell_at(x + 1, y - 1).type, // bottom left
    grid.cell_at(x + 1, y).type,     // bottom
    grid.cell_at(x + 1, y + 1).type  // bottom right
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
    grid.set_next(x, y, { .type = CellType::AIR });
    grid.set_next(x + 1, y, { .type = CellType::SAND });
  }

  /* else flow left if possible */
  else if (IS_FLUID(context.bottom_left)) {
    grid.set_next(x, y, { .type = CellType::AIR });
    grid.set_next(x + 1, y - 1, { .type = CellType::SAND });
  }

  /* else flow right if possible */
  else if (IS_FLUID(context.bottom_right)) {
    grid.set_next(x, y, { .type = CellType::AIR });
    grid.set_next(x + 1, y + 1, { .type = CellType::SAND });
  }
}

void FireCell::helper(CellType curr, Grid &grid, int x, int y, float remaining_mass) {
  if (IS_FLAMMABLE(curr)) {
    grid.set_next(x, y, { .type = CellType::FIRE, .mass = remaining_mass });
  } else if (curr == CellType::AIR) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    float p = 0.4;
    double rand_num = dis(gen);
    if (rand_num < p) {
      grid.set_next(x, y, { .type = CellType::SMOKE, .mass = remaining_mass - mass_decay });
    }
  }
}

void FireCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  float fire_mass = grid.cell_at(x, y).mass;
  float remaining_mass = fire_mass - mass_decay;

  if (remaining_mass <= 0.0f) {
    grid.set_next(x, y, { .type = CellType::SMOKE, .mass = 0.0f } );
    return;
  }

  FireCell::helper(context.top, grid, x - 1, y, remaining_mass);
  FireCell::helper(context.top_left, grid, x - 1, y - 1, remaining_mass);
  FireCell::helper(context.top_right, grid, x - 1, y + 1, remaining_mass);

  FireCell::helper(context.bottom, grid, x + 1, y, remaining_mass);
  FireCell::helper(context.bottom_left, grid, x + 1, y - 1, remaining_mass);
  FireCell::helper(context.bottom_right, grid, x + 1, y + 1, remaining_mass);

  FireCell::helper(context.left, grid, x, y - 1, remaining_mass);
  FireCell::helper(context.right, grid, x, y + 1, remaining_mass);

  grid.set_next(x, y, { .type = CellType::FIRE, .mass = std::max(0.0f, remaining_mass)});
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
        grid.set_next(x, y, { .type = CellType::AIR });
        grid.set_next(x - 1, y, { .type = CellType::SMOKE });
    }
  }

  /* else flow left if possible */
  else if (IS_FLUID(context.top_left)) {
    if (rand_num < p) {
      grid.set_next(x, y, { .type = CellType::AIR });
      grid.set_next(x - 1, y - 1, { .type = CellType::SMOKE });
    }
  }

  /* else flow right if possible */
  else if (IS_FLUID(context.top_right)) {
    if (rand_num < p) {
      grid.set_next(x, y, { .type = CellType::AIR });
      grid.set_next(x - 1, y + 1, { .type = CellType::SMOKE });
    }
  }
}

// TODO(joe): water broken, need to fix
void WaterCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  float flow;
  float remaining_mass = grid.cell_at(x, y).mass;
  if (remaining_mass <= 0) {
    grid.set_next(x, y, { .type = CellType::AIR });
    return;
  }

  // Flow to block below.
  if (IS_FLUID(context.bottom)) {
    // Convert to water.
    grid.set_next(x + 1, y, { .type = CellType::WATER });

    flow = get_stable_state_b(remaining_mass + grid.cell_at(x + 1, y).mass)
        - grid.cell_at(x + 1, y).mass;

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, std::min(max_speed, remaining_mass));

    cell_data_t cell = grid.cell_at(x, y);
    cell.mass -= flow;
    grid.set_next(x, y, cell);

    cell = grid.cell_at(x + 1, y);
    cell.mass += flow;
    grid.set_next(x + 1, y, cell);

    remaining_mass -= flow;
  }

  if (remaining_mass <= 0) {
    grid.set_next(x, y, { .type = CellType::AIR });
    return;
  }

  // Equalize water with right block.
  if (IS_FLUID(context.right)) {
    // Convert to water.
    grid.set_next(x, y + 1, { .type = CellType::WATER });

    flow = (grid.cell_at(x, y).mass - grid.cell_at(x, y + 1).mass) / 4;

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, remaining_mass);

    cell_data_t cell = grid.cell_at(x, y);
    cell.mass -= flow;
    grid.set_next(x, y, cell);

    cell = grid.cell_at(x, y + 1);
    cell.mass += flow;
    grid.set_next(x, y + 1, cell);

    remaining_mass -= flow;
  }

  if (remaining_mass <= 0) {
    grid.set_next(x, y, { .type = CellType::AIR });
    return;
  }

  // Equalize water with left block.
  if (IS_FLUID(context.left)) {
    // Convert to water.
    grid.set_next(x, y - 1, { .type = CellType::WATER });

    flow = (grid.cell_at(x, y).mass - grid.cell_at(x, y - 1).mass) / 4;

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, remaining_mass);

    cell_data_t cell = grid.cell_at(x, y);
    cell.mass -= flow;
    grid.set_next(x, y, cell);

    cell = grid.cell_at(x, y - 1);
    cell.mass += flow;
    grid.set_next(x, y - 1, cell);

    remaining_mass -= flow;
  }

  if (remaining_mass <= 0) {
    grid.set_next(x, y, { .type = CellType::AIR });
    return;
  }

  // If compressed, flow up.
  if (IS_FLUID(context.top)) {
    // Convert to water.
    grid.set_next(x, y, { .type = CellType::WATER });

    flow = remaining_mass
        - get_stable_state_b(remaining_mass + grid.cell_at(x - 1, y).mass);

    // Smoother flow.
    if (flow > min_flow) flow *= 0.5;

    flow = constrain(flow, 0, std::min(max_speed, remaining_mass));

    cell_data_t cell = grid.cell_at(x, y);
    cell.mass -= flow;
    grid.set_next(x, y, cell);

    cell = grid.cell_at(x - 1, y);
    cell.mass += flow;
    grid.set_next(x - 1, y, cell);

    remaining_mass -= flow;
  }
}
} /* namespace simulake */
