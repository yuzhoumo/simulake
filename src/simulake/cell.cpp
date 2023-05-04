#include <iostream>
#include <utility>

#include "utils.hpp"

#include "cell.hpp"
#include "grid.hpp"

namespace simulake {

std::random_device BaseCell::rd;
std::mt19937 BaseCell::gen(BaseCell::rd());

bool BaseCell::is_liquid(CellType type) {
  return type == CellType::WATER or type == CellType::OIL or type == CellType::NAPALM;
}

bool BaseCell::is_gas(CellType type) {
  return type == CellType::AIR or type == CellType::SMOKE or
         type == CellType::FIRE; // not technically gas but whatever
}

bool BaseCell::is_fluid(CellType type) {
  return is_liquid(type) or is_gas(type);
}

float BaseCell::flammability(CellType type) {
  switch (type) {
  case CellType::AIR:
    return 0.0f;
  case CellType::WATER:
    return 0.0f;
  case CellType::OIL:
    return 1.0f;
  case CellType::SAND:
    return 0.5f;
  case CellType::FIRE:
    return 0.0f;
  case CellType::NAPALM:
    return 0.0f;
  case CellType::SMOKE:
    return 0.0f;
  case CellType::STONE:
    return 0.0f;
  default:
    return 0.0f;
  };
}

/* compute neighbor types and return as 8 element tuple */
BaseCell::context_t BaseCell::get_cell_context(const BaseCell::position_t &pos,
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

float BaseCell::random_float(float lower, float upper) {
  std::uniform_real_distribution<float> dist(lower, upper);
  return dist(gen);
}

int BaseCell::random_int(int lower, int upper) {
  std::uniform_int_distribution<int> dist(lower, upper);
  return dist(gen);
}

/* <<<<<<<< AIR >>>>>>>>> */

cell_data_t AirCell::spawn(const position_t &pos, Grid &grid) noexcept {
  return {
      .type = CellType::AIR,
      .updated = true,
  };
}

void AirCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);
}

/* <<<<<<<< SMOKE >>>>>>>>> */

cell_data_t SmokeCell::spawn(const position_t &pos, Grid &grid) noexcept {
  return {
      .type = CellType::SMOKE,
      .mass = 1.0f,
      .updated = true,
  };
}

void SmokeCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = get_cell_context(pos, grid);
  const auto curr = grid.cell_at(x, y);

  bool decayed = curr.mass <= 0.0f;

  /* generate a random number between 0 and 1 */
  float p = 0.4;
  float rand_num = random_float(0.0f, 1.0f);

  if (decayed) {
    grid.set_next(x, y, {.type = CellType::AIR});
  }

  /* will flow up if possible */
  else if (is_fluid(context.top)) {
    if (rand_num < p) {
      grid.set_next(x, y, {.type = CellType::AIR});
      grid.set_next(x, y - 1,
                    {.type = CellType::SMOKE, .mass = curr.mass - mass_decay});
    }
  }

  /* else flow left if possible */
  else if (is_fluid(context.top_left)) {
    if (rand_num < p) {
      grid.set_next(x, y, {.type = CellType::AIR});
      grid.set_next(x - 1, y - 1,
                    {.type = CellType::SMOKE, .mass = curr.mass - mass_decay});
    }
  }

  /* else flow right if possible */
  else if (is_fluid(context.top_right)) {
    if (rand_num < p) {
      grid.set_next(x, y, {.type = CellType::AIR});
      grid.set_next(x + 1, y - 1,
                    {.type = CellType::SMOKE, .mass = curr.mass - mass_decay});
    }
  }

  /* else stuck, decay in place */
  else {
    grid.set_next(x, y,
                  {.type = CellType::SMOKE, .mass = curr.mass - mass_decay});
  }
}

/* <<<<<<<< FIRE >>>>>>>> */

cell_data_t FireCell::spawn(const position_t &pos, Grid &grid) noexcept {
  return {
      .type = CellType::FIRE,
      .mass = random_float(0.6f, 1.0f),
      .updated = true,
  };
}

void FireCell::helper(CellType curr, Grid &grid, int x, int y,
                      float remaining_mass) {
  if (flammability(curr) > 0) {
    grid.set_next(x, y, {.type = CellType::FIRE, .mass = remaining_mass});
  } else if (curr == CellType::AIR) {
    float p = 0.4;
    float rand_num = random_float(0.0f, 1.0f);
    if (rand_num < p) {
      grid.set_next(
          x, y, {.type = CellType::SMOKE, .mass = remaining_mass - mass_decay});
    }
  }
}

void FireCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);

  float fire_mass = grid.cell_at(x, y).mass;
  float remaining_mass = fire_mass - mass_decay;

  if (remaining_mass <= 0.0f) {
    grid.set_next(x, y, {.type = CellType::SMOKE, .mass = 0.0f});
    return;
  }

  FireCell::helper(context.top, grid, x, y - 1, remaining_mass);
  FireCell::helper(context.top_left, grid, x - 1, y - 1, remaining_mass);
  FireCell::helper(context.top_right, grid, x + 1, y - 1, remaining_mass);

  FireCell::helper(context.bottom, grid, x, y + 1, remaining_mass);
  FireCell::helper(context.bottom_left, grid, x - 1, y + 1, remaining_mass);
  FireCell::helper(context.bottom_right, grid, x + 1, y + 1, remaining_mass);

  FireCell::helper(context.left, grid, x - 1, y, remaining_mass);
  FireCell::helper(context.right, grid, x + 1, y, remaining_mass);

  grid.set_next(
      x, y, {.type = CellType::FIRE, .mass = std::max(0.0f, remaining_mass)});
}

/* <<<<<<<< WATER >>>>>>>> */

cell_data_t WaterCell::spawn(const position_t &pos, Grid &grid) noexcept {
  return {
    .type = CellType::WATER,
    .mass = max_mass,
    .updated = true,
  };
}


void WaterCell::step(const position_t &pos, Grid &grid) noexcept{

}

cell_data_t NapalmCell::spawn(const position_t &, Grid &) noexcept {
  return {
    .type = CellType::NAPALM,
    .mass = 1.0f,
    .updated = true,
  };
}

void NapalmCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;

  cell_data_t current_cell = grid.cell_at(x, y);


  // Check downward movement
  cell_data_t below_cell = grid.cell_at(x, y + 1);

  if (below_cell.type == CellType::NAPALM and random_int(0, 1000) == 0) {
    // spontaneously combust with varying intensity
    grid.set_next(x, y, { .type = CellType::FIRE, .mass = random_float(0.0, 4.0) });
    return;
  }

  if (!is_fluid(below_cell.type) and random_int(0, 10) == 0) {
    // spontaneously combust with varying intensity
    grid.set_next(x, y, { .type = CellType::FIRE, .mass = random_float(0.0, 2.0) });
    return;
  }

  if (!below_cell.updated and (below_cell.type == CellType::AIR or below_cell.type == CellType::SMOKE)) {
    // mark updated
    grid.mark_updated(x, y + 1);

    // Water moves down
    grid.set_next(x, y + 1, current_cell);
    grid.set_next(x, y, { .type = CellType::AIR, .mass = 0.0f, .updated = false });
    return;
  }

  // Check lateral movement (left and right)
  int direction = random_int(-1, 1);

  cell_data_t side_cell = grid.cell_at(x + direction, y);
  if (!side_cell.updated and (side_cell.type == CellType::AIR or side_cell.type == CellType::SMOKE)) {
    // mark updated
    grid.mark_updated(x + direction, y);

    // Water moves laterally
    grid.set_next(x + direction, y, current_cell);
    grid.set_next(x, y, {.type = CellType::AIR, .mass = 0.0f, .updated = false});
    return;
  }

  // Water remains in the current cell
  grid.set_next(x, y, current_cell);
}

// void WaterCell::step(const position_t &pos, Grid &grid) noexcept {
//   const auto [x, y] = pos;
//   const auto context = BaseCell::get_cell_context(pos, grid);
//
//   float flow;
//   float remaining_mass = grid.cell_at(x, y).mass;
//   if (remaining_mass <= 0) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     return;
//   }
//
//   // Flow to block below.
//   if (is_fluid(context.bottom)) {
//     flow = get_stable_state_b(remaining_mass + grid.cell_at(x, y + 1).mass) - grid.cell_at(x, y + 1).mass;
//
//     // Smoother flow.
//     if (flow > min_flow) flow *= 0.5;
//
//     flow = constrain(flow, 0, std::min(max_speed, remaining_mass));
//
//     // flow into neighbor, add to mass of neighbor, remove mass from current
//     grid.set_next(x, y + 1, { .type = CellType::WATER, .mass = grid.cell_at(x, y + 1, true).mass + flow });
//     grid.set_next(x, y, { .type = CellType::WATER, .mass = grid.cell_at(x, y, true).mass - flow });
//
//     remaining_mass -= flow;
//   }
//
//   if (remaining_mass <= 0) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     return;
//   }
//
//   // Equalize water with right block.
//   if (is_fluid(context.right)) {
//     flow = (grid.cell_at(x, y).mass - grid.cell_at(x + 1, y).mass) / 4;
//
//     // Smoother flow.
//     if (flow > min_flow) flow *= 0.5;
//
//     flow = constrain(flow, 0, remaining_mass);
//
//     grid.set_next(x + 1, y, { .type = CellType::WATER, .mass = grid.cell_at(x + 1, y, true).mass + flow });
//     grid.set_next(x, y, { .type = CellType::WATER, .mass = grid.cell_at(x, y, true).mass - flow });
//
//     remaining_mass -= flow;
//   }
//
//   if (remaining_mass <= 0) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     return;
//   }
//
//   // Equalize water with left block.
//   if (is_fluid(context.left)) {
//     flow = (grid.cell_at(x, y).mass - grid.cell_at(x - 1, y).mass) / 4;
//
//     // Smoother flow.
//     if (flow > min_flow) flow *= 0.5;
//
//     flow = constrain(flow, 0, remaining_mass);
//
//     grid.set_next(x - 1, y, { .type = CellType::WATER, .mass = grid.cell_at(x - 1, y, true).mass + flow });
//     grid.set_next(x, y, { .type = CellType::WATER, .mass = grid.cell_at(x, y, true).mass - flow });
//
//
//     remaining_mass -= flow;
//   }
//
//   if (remaining_mass <= 0) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     return;
//   }
//
//   // If compressed, flow up.
//   if (is_fluid(context.top)) {
//     flow = remaining_mass - get_stable_state_b(remaining_mass + grid.cell_at(x, y - 1).mass);
//
//     // Smoother flow.
//     if (flow > min_flow) flow *= 0.5;
//
//     flow = constrain(flow, 0, std::min(max_speed, remaining_mass));
//
//     grid.set_next(x, y - 1, { .type = CellType::WATER, .mass = grid.cell_at(x, y - 1, true).mass + flow });
//     grid.set_next(x, y, { .type = CellType::WATER, .mass = grid.cell_at(x, y, true).mass - flow });
//
//     remaining_mass -= flow;
//   }
// }

/* <<<<<<<< SAND >>>>>>>> */

cell_data_t SandCell::spawn(const position_t &pos, Grid &grid) noexcept {
  return {
      .type = CellType::SAND,
      .updated = true,
  };
}

void SandCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto dt = grid.get_delta_time();
  const auto [x, y] = pos;

  cell_data_t new_cell = grid.cell_at(x, y);

  // gravity is effectively 50 for sand, mult by 3 to fix flying horizontally
  new_cell.velocity.y =
      std::clamp(new_cell.velocity.y + (gravity * 5.f * dt), -10.f, 10.f);

  // reset velocity if not able to move directly below
  if (grid.in_bounds(x, y + 1) and !grid.is_empty(x, y + 1) and
      !is_liquid(grid.cell_at(x, y + 1).type)) {
    new_cell.velocity.y /= 2.f;
  }

  glm::ivec2 vi{static_cast<int>(x) + static_cast<int>(new_cell.velocity.x),
                static_cast<int>(y) + static_cast<int>(new_cell.velocity.y)};

  // check if can swap with element below
  glm::ivec2 bottom{x, y + 1};
  glm::ivec2 bottom_right{x + 1, y + 1};
  glm::ivec2 bottom_left{x - 1, y + 1};

  cell_data_t tmp_a = grid.cell_at(x, y);

  // physics (using velocity)
  if (grid.in_bounds(vi.x, vi.y) and
      ((grid.is_empty(vi.x, vi.y) ||
        (((grid.cell_at(vi.x, vi.y).type == CellType::WATER) and
          !grid.cell_at(vi.x, vi.y).updated and
          glm::length(grid.cell_at(vi.x, vi.y).velocity) -
                  glm::length(tmp_a.velocity) >
              10.f))))) {

    cell_data_t tmp_b = grid.cell_at(vi.x, vi.y);

    // try to throw water out
    if (tmp_b.type == CellType::WATER) {

      int rx = random_int(-2, 2);
      tmp_b.velocity = glm::vec2{rx, -4.f};

      grid.set_next(vi.x, vi.y, tmp_a);

      for (int i = -10; i < 0; ++i) {
        for (int j = -10; j < 10; ++j) {
          if (grid.is_empty(vi.x + j, vi.y + i)) {
            grid.set_next(vi.x + j, vi.y + i, tmp_b);
            break;
          }
        }
      }

      // couldn't write here, destroy it
      grid.set_next(x, y, cell_data_t{.type = CellType::AIR});
    } else if (grid.is_empty(vi.x, vi.y)) {
      grid.set_next(vi.x, vi.y, tmp_a);
      grid.set_next(x, y, tmp_b);
    }
  }
  // simple falling
  else if (grid.in_bounds(x, y + 1) &&
           ((grid.is_empty(x, y + 1) or
             (grid.cell_at(bottom.x, bottom.y).type == CellType::WATER)))) {
    new_cell.velocity.y += (gravity * dt);
    cell_data_t tmp_b = grid.cell_at(x, y + 1);
    grid.set_next(bottom.x, bottom.y, new_cell);
    grid.set_next(x, y, tmp_b);
  } else if (grid.in_bounds(x - 1, y + 1) and
             ((grid.is_empty(x - 1, y + 1) or
               grid.cell_at(bottom_right.x, bottom_right.y).type ==
                   CellType::WATER))) {
    new_cell.velocity.x =
        grid.is_in_liquid(x, y)[0] == 1 ? 0.f : random_float(-2, 2);
    new_cell.velocity.y += (gravity * dt);
    cell_data_t tmp_b = grid.cell_at(x - 1, y + 1);
    grid.set_next(bottom_left.x, bottom_left.y, new_cell);
    grid.set_next(x, y, tmp_b);
  } else if (grid.in_bounds(x + 1, y + 1) &&
             ((grid.is_empty(x + 1, y + 1) or
               grid.cell_at(bottom_right.x, bottom_right.y).type ==
                   CellType::WATER))) {
    new_cell.velocity.x =
        grid.is_in_liquid(x, y)[0] == 1 ? 0.f : random_float(-2, 2);
    new_cell.velocity.y += (gravity * dt);
    cell_data_t tmp_b = grid.cell_at(x + 1, y + 1);
    grid.set_next(bottom_right.x, bottom_right.y, new_cell);
    grid.set_next(x, y, tmp_b);
  } else if (random_int(0, 10) == 0) {
    glm::vec3 in_liquid = grid.is_in_liquid(x, y);
    if (in_liquid[0] == 1) {
      cell_data_t tmp_b = grid.cell_at(in_liquid[1], in_liquid[2]);
      grid.set_next(in_liquid[1], in_liquid[2], new_cell);
      grid.set_next(x, y, tmp_b);
    }
  }
}

/* <<<<<<<< STONE >>>>>>>> */

cell_data_t StoneCell::spawn(const position_t &pos, Grid &grid) noexcept {
  return {
      .type = CellType::STONE,
      .updated = true,
  };
}

void StoneCell::step(const position_t &pos, Grid &grid) noexcept {
  const auto [x, y] = pos;
  const auto context = BaseCell::get_cell_context(pos, grid);
}

// void SandCell::step(const position_t &pos, Grid &grid) noexcept {
//   const auto [x, y] = pos;
//   const auto context = BaseCell::get_cell_context(pos, grid);
//
//   /* will flow down if possible */
//   if (IS_FLUID(context.bottom)) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     grid.set_next(x, y + 1, { .type = CellType::SAND });
//   }
//
//   /* else flow left if possible */
//   else if (IS_FLUID(context.bottom_left)) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     grid.set_next(x - 1, y + 1, { .type = CellType::SAND });
//   }
//
//   /* else flow right if possible */
//   else if (IS_FLUID(context.bottom_right)) {
//     grid.set_next(x, y, { .type = CellType::AIR });
//     grid.set_next(x + 1, y + 1, { .type = CellType::SAND });
//   }
// }


} /* namespace simulake */
