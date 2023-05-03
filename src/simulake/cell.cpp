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
    grid.cell_at(x, y - 1).type,     // top
    grid.cell_at(x + 1, y - 1).type, // top right
    grid.cell_at(x - 1, y).type,     // left
    grid.cell_at(x + 1, y).type,     // right
    grid.cell_at(x - 1, y + 1).type, // bottom left
    grid.cell_at(x, y + 1).type,     // bottom
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
  const auto dt = grid.get_delta_time();
  const auto [x, y] = pos;

  cell_data_t new_cell = grid.cell_at(x, y);

  new_cell.velocity.y =
      std::clamp(new_cell.velocity.y + (gravity * dt), -10.f, 10.f);

  // Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
  if (grid.in_bounds(x, y + 1) && !grid.is_empty(x, y + 1) && grid.cell_at(x, y + 1).type != CellType::WATER) {
    new_cell.velocity.y /= 2.f;
  }

  glm::ivec2 vi{
      static_cast<int>(x) + static_cast<int>(new_cell.velocity.x),
      static_cast<int>(y) + static_cast<int>(new_cell.velocity.y)};

  // Check to see if you can swap first with other element below you
  glm::ivec2 bottom{x, y + 1}; // TODO make sure this doesn't fuck up uint32
  glm::ivec2 bottom_right{x + 1, y + 1};
  glm::ivec2 bottom_left{x - 1, y + 1};

  cell_data_t tmp_a = grid.cell_at(x, y);

  // Physics (using velocity)
  if (grid.in_bounds(vi.x, vi.y) and ((grid.is_empty(vi.x, vi.y) ||
        (((grid.cell_at(vi.x, vi.y).type == CellType::WATER) and
          !grid.cell_at(vi.x, vi.y).updated && glm::length(grid.cell_at(vi.x, vi.y).velocity) - glm::length(tmp_a.velocity) > 10.f))))) {

    cell_data_t tmp_b = grid.cell_at(vi.x, vi.y);

    // Try to throw water out
    if (tmp_b.type == CellType::WATER) {

      int rx = random_int(-2, 2);
      tmp_b.velocity = glm::vec2{rx, -4.f};

      grid.set_next(vi.x, vi.y, tmp_a);

      for(int i = -10; i < 0; ++i) {
        for (int j = -10; j < 10; ++j) {
          if (grid.is_empty(vi.x + j, vi.y + i)) {
            grid.set_next(vi.x + j, vi.y + i, tmp_b);
            break;
          }
        }
      }

      // Couldn't write there, so, uh, destroy it.
      grid.set_next(x, y, cell_data_t{ .type = CellType::AIR });
    }
    else if (grid.is_empty(vi.x, vi.y)) {
      grid.set_next(vi.x, vi.y, tmp_a);
      grid.set_next(x, y, tmp_b);
    }
  }
  // Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
  else if (grid.in_bounds(x, y + 1) && ((grid.is_empty(x, y + 1) || (grid.cell_at(bottom.x, bottom.y).type == CellType::WATER)))) {
    new_cell.velocity.y += (gravity * dt);
    cell_data_t tmp_b = grid.cell_at(x, y + 1);
    grid.set_next(bottom.x, bottom.y, new_cell);
    grid.set_next(x, y, tmp_b);
  }
  else if (grid.in_bounds(x - 1, y + 1) and ((grid.is_empty(x - 1, y + 1) || grid.cell_at(bottom_right.x, bottom_right.y).type == CellType::WATER))) {
    new_cell.velocity.x = grid.is_in_liquid(x, y)[0] == 1 ? 0.f : random_int(0, 1) == 0 ? -.5f : .5f;
    new_cell.velocity.y += (gravity * dt);
    cell_data_t tmp_b = grid.cell_at(x - 1, y + 1);
    grid.set_next(bottom_left.x, bottom_left.y, new_cell);
    grid.set_next(x, y, tmp_b);
  }
  else if (grid.in_bounds(x + 1, y + 1) && ((grid.is_empty(x + 1, y + 1) || grid.cell_at(bottom_right.x, bottom_right.y).type == CellType::WATER))) {
    new_cell.velocity.x = grid.is_in_liquid(x, y)[0] == 1 ? 0.f : random_int(0, 1) == 0 ? -.5f : .5f;
    new_cell.velocity.y += (gravity * dt);
    cell_data_t tmp_b = grid.cell_at(x + 1, y + 1);
    grid.set_next(bottom_right.x, bottom_right.y, new_cell);
    grid.set_next(x, y, tmp_b);
  }
  else if (random_int(0, 10) == 0) {
    glm::vec3 in_liquid = grid.is_in_liquid(x, y);
    if (in_liquid[0] == 1) {
      cell_data_t tmp_b = grid.cell_at(in_liquid[1], in_liquid[2]);
      grid.set_next(in_liquid[1], in_liquid[2], new_cell);
      grid.set_next(x, y, tmp_b);
    }
  }
}

// void SandCell::step(const position_t &pos, Grid &grid) noexcept {
//   const auto [x, y] = pos;
//   const auto context = BaseCell::get_cell_context(pos, grid);
//
//   /* will flow down if possible */
//   if (IS_FLUID(context.bottom)) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     grid.set_next(x + 1, y, { .type = CellType::SAND });
//   }
//
//   /* else flow left if possible */
//   else if (IS_FLUID(context.bottom_left)) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     grid.set_next(x + 1, y - 1, { .type = CellType::SAND });
//   }
//
//   /* else flow right if possible */
//   else if (IS_FLUID(context.bottom_right)) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     grid.set_next(x + 1, y + 1, { .type = CellType::SAND });
//   }
// }

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
  // const auto [x, y] = pos;
  // const auto context = BaseCell::get_cell_context(pos, grid);
  //
  // float flow;
  // float remaining_mass = grid.cell_at(x, y).mass;
  // if (remaining_mass <= 0) {
  //   grid.set_next(x, y, { .type = CellType::AIR });
  //   return;
  // }
  //
  // // Flow to block below.
  // if (IS_FLUID(context.bottom)) {
  //   // Convert to water.
  //   grid.set_next(x + 1, y, { .type = CellType::WATER });
  //
  //   flow = get_stable_state_b(remaining_mass + grid.cell_at(x + 1, y).mass)
  //       - grid.cell_at(x + 1, y).mass;
  //
  //   // Smoother flow.
  //   if (flow > min_flow) flow *= 0.5;
  //
  //   flow = constrain(flow, 0, std::min(max_speed, remaining_mass));
  //
  //   cell_data_t cell = grid.cell_at(x, y);
  //   cell.mass -= flow;
  //   grid.set_next(x, y, cell);
  //
  //   cell = grid.cell_at(x + 1, y);
  //   cell.mass += flow;
  //   grid.set_next(x + 1, y, cell);
  //
  //   remaining_mass -= flow;
  // }
  //
  // if (remaining_mass <= 0) {
  //   grid.set_next(x, y, { .type = CellType::AIR });
  //   return;
  // }
  //
  // // Equalize water with right block.
  // if (IS_FLUID(context.right)) {
  //   // Convert to water.
  //   grid.set_next(x, y + 1, { .type = CellType::WATER });
  //
  //   flow = (grid.cell_at(x, y).mass - grid.cell_at(x, y + 1).mass) / 4;
  //
  //   // Smoother flow.
  //   if (flow > min_flow) flow *= 0.5;
  //
  //   flow = constrain(flow, 0, remaining_mass);
  //
  //   cell_data_t cell = grid.cell_at(x, y);
  //   cell.mass -= flow;
  //   grid.set_next(x, y, cell);
  //
  //   cell = grid.cell_at(x, y + 1);
  //   cell.mass += flow;
  //   grid.set_next(x, y + 1, cell);
  //
  //   remaining_mass -= flow;
  // }
  //
  // if (remaining_mass <= 0) {
  //   grid.set_next(x, y, { .type = CellType::AIR });
  //   return;
  // }
  //
  // // Equalize water with left block.
  // if (IS_FLUID(context.left)) {
  //   // Convert to water.
  //   grid.set_next(x, y - 1, { .type = CellType::WATER });
  //
  //   flow = (grid.cell_at(x, y).mass - grid.cell_at(x, y - 1).mass) / 4;
  //
  //   // Smoother flow.
  //   if (flow > min_flow) flow *= 0.5;
  //
  //   flow = constrain(flow, 0, remaining_mass);
  //
  //   cell_data_t cell = grid.cell_at(x, y);
  //   cell.mass -= flow;
  //   grid.set_next(x, y, cell);
  //
  //   cell = grid.cell_at(x, y - 1);
  //   cell.mass += flow;
  //   grid.set_next(x, y - 1, cell);
  //
  //   remaining_mass -= flow;
  // }
  //
  // if (remaining_mass <= 0) {
  //   grid.set_next(x, y, { .type = CellType::AIR });
  //   return;
  // }
  //
  // // If compressed, flow up.
  // if (IS_FLUID(context.top)) {
  //   // Convert to water.
  //   grid.set_next(x, y, { .type = CellType::WATER });
  //
  //   flow = remaining_mass
  //       - get_stable_state_b(remaining_mass + grid.cell_at(x - 1, y).mass);
  //
  //   // Smoother flow.
  //   if (flow > min_flow) flow *= 0.5;
  //
  //   flow = constrain(flow, 0, std::min(max_speed, remaining_mass));
  //
  //   cell_data_t cell = grid.cell_at(x, y);
  //   cell.mass -= flow;
  //   grid.set_next(x, y, cell);
  //
  //   cell = grid.cell_at(x - 1, y);
  //   cell.mass += flow;
  //   grid.set_next(x - 1, y, cell);
  //
  //   remaining_mass -= flow;
  // }
}

} /* namespace simulake */
