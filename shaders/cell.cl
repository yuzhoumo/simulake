// vim: ft=cpp :

#ifndef SIMULAKE_COMPUTE_CELL_CL
#define SIMULAKE_COMPUTE_CELL_CL

// base macros and definitions
#include "base.cl"

STEP_IMPL(smoke_step) {
  GEN_STEP_IMPL_HEADER();

#define __move_smoke__(idx_next)                                               \
  {                                                                            \
    next_grid[idx].type = AIR_TYPE;                                            \
    next_grid[idx].mass = AIR_MASS;                                            \
    next_grid[idx].velocity = V_STATIONARY;                                    \
    next_grid[idx].updated = false;                                            \
    next_grid[idx_next].type = SMOKE_TYPE;                                     \
    next_grid[idx_next].mass = grid[idx].mass - mass_decay;                    \
    next_grid[idx_next].velocity = grid[idx].velocity;                         \
    next_grid[idx_next].updated = false;                                       \
    grid[idx].updated = true;                                                  \
    grid[idx_next].updated = true;                                             \
    moved = true;                                                              \
  }

  const float p = 0.4f;
  const float min_mass = 0.0f;
  const float mass_decay = 0.015f;

  const bool decayed = grid[idx].mass <= min_mass;
  bool moved = false;

  if (decayed) {
    next_grid[idx].type = AIR_TYPE;
    next_grid[idx].mass = AIR_MASS;
    next_grid[idx].velocity = V_STATIONARY;
    next_grid[idx].updated = false;
    grid[idx].updated = true;

    moved = true;
  }

  else if (top_valid) {
    if (get_rand_float(seed) > p)
      return;

    // clang-format off
    if      (               IS_FLUID(grid[idx_top]))       { __move_smoke__(idx_top);       }
    else if (left_valid  && IS_FLUID(grid[idx_top_left]))  { __move_smoke__(idx_top_left);  }
    else if (right_valid && IS_FLUID(grid[idx_top_right])) { __move_smoke__(idx_top_right); }
    // clang-format on
  }

  // decay in place
  if (!moved) {
    next_grid[idx].mass = grid[idx].mass - mass_decay;
    next_grid[idx].updated = false;
    grid[idx].updated = true;
  }
}

STEP_IMPL(fire_step) {
  GEN_STEP_IMPL_HEADER();

#define __to_fire_or_smoke__(target, r, c)                                     \
  if (IS_FLAMMABLE(grid[target])) {                                            \
    next_grid[target].type = FIRE_TYPE;                                        \
    next_grid[target].mass = remaining_mass;                                   \
    next_grid[target].velocity = V_STATIONARY;                                 \
    next_grid[target].updated = false;                                         \
    grid[idx].updated = true;                                                  \
  } else if (IS_AIR(grid[target]) && get_rand_float(seed) < p) {               \
    next_grid[target].type = SMOKE_TYPE;                                       \
    next_grid[target].mass = remaining_mass - mass_decay;                      \
    next_grid[target].velocity = grid[idx].velocity;                           \
    next_grid[target].updated = false;                                         \
    grid[idx].updated = true;                                                  \
  }

  const float p = 0.4f;
  const float min_mass = 0.0f;
  const float mass_decay = 0.05f;

  const float remaining_mass = grid[idx].mass - mass_decay;

  if (remaining_mass <= min_mass) {
    next_grid[idx].type = SMOKE_TYPE;
    next_grid[idx].mass = SMOKE_MASS;
    next_grid[idx].velocity = grid[idx].velocity;
    next_grid[idx].updated = false;
    grid[idx].updated = true;
    return;
  }

  // clang-format off
  if (top_valid) {
                       __to_fire_or_smoke__(idx_top,       (long) row - 1, (long) col + 0);
    if (left_valid)  { __to_fire_or_smoke__(idx_top_left,  (long) row - 1, (long) col - 1); }
    if (right_valid) { __to_fire_or_smoke__(idx_top_right, (long) row - 1, (long) col + 1); }
  }

  if (bot_valid) {
                       __to_fire_or_smoke__(idx_bot,       (long) row + 1, (long) col + 0);
    if (left_valid)  { __to_fire_or_smoke__(idx_bot_left,  (long) row + 1, (long) col - 1); }
    if (right_valid) { __to_fire_or_smoke__(idx_bot_right, (long) row + 1, (long) col + 1); }
  }
  // clang-format on

  next_grid[idx].type = FIRE_TYPE;
  next_grid[idx].mass = fmax(0.0f, remaining_mass);
  next_grid[idx].velocity = V_STATIONARY;
  next_grid[idx].updated = false;
  grid[idx].updated = true;
}

STEP_IMPL(greek_fire_step) {
  GEN_STEP_IMPL_HEADER();

#define __to_greek_fire_or_smoke__(target, r, c)                               \
  if (IS_FLAMMABLE(grid[target])) {                                            \
    next_grid[target].type = FIRE_TYPE;                                        \
    next_grid[target].mass = remaining_mass;                                   \
    next_grid[target].velocity = V_STATIONARY;                                 \
    next_grid[target].updated = false;                                         \
    grid[idx].updated = true;                                                  \
  } else if (IS_AIR(grid[target]) && get_rand_float(seed) < p) {               \
    next_grid[target].type = SMOKE_TYPE;                                       \
    next_grid[target].mass = get_mass(SMOKE_TYPE, seed);                       \
    next_grid[target].velocity = grid[idx].velocity;                           \
    next_grid[target].updated = false;                                         \
    grid[idx].updated = true;                                                  \
  }

  const float p = 0.4f;
  const float min_mass = 0.0f;
  const float mass_decay = 0.05f;

  const float remaining_mass = grid[idx].mass - mass_decay;

  if (remaining_mass <= min_mass) {
    next_grid[idx].type = GREEK_FIRE_TYPE;
    next_grid[idx].mass = get_mass(GREEK_FIRE_TYPE, seed);
    next_grid[idx].velocity = grid[idx].velocity;
    next_grid[idx].updated = false;
    grid[idx].updated = true;
    return;
  }

  // clang-format off
  if (top_valid) {
                       __to_greek_fire_or_smoke__(idx_top,       (long) row - 1, (long) col + 0);
    if (left_valid)  { __to_greek_fire_or_smoke__(idx_top_left,  (long) row - 1, (long) col - 1); }
    if (right_valid) { __to_greek_fire_or_smoke__(idx_top_right, (long) row - 1, (long) col + 1); }
  }

  if (bot_valid) {
                       __to_greek_fire_or_smoke__(idx_bot,       (long) row + 1, (long) col + 0);
    if (left_valid)  { __to_greek_fire_or_smoke__(idx_bot_left,  (long) row + 1, (long) col - 1); }
    if (right_valid) { __to_greek_fire_or_smoke__(idx_bot_right, (long) row + 1, (long) col + 1); }
  }
  // clang-format on

  next_grid[idx].type = GREEK_FIRE_TYPE;
  next_grid[idx].mass = fmax(0.0f, remaining_mass);
  next_grid[idx].velocity = V_STATIONARY;
  next_grid[idx].updated = false;
  grid[idx].updated = true;
}

STEP_IMPL(jet_fuel_step) {
  GEN_STEP_IMPL_HEADER();

  grid[idx].updated = true;
  next_grid[idx].updated = false;

  if (top_valid && IS_FLAMMABLE(grid[idx_top]) && get_rand(seed) % 10 < 3) {
    next_grid[idx].type = FIRE_TYPE;
    next_grid[idx].mass = SCALE_FLOAT(get_rand_float(seed), 0.7f, 4.0f);
    next_grid[idx].velocity = V_STATIONARY;
    return;
  }

  if (top_valid && IS_WATER(grid[idx_top])) {
    next_grid[idx].type = SMOKE_TYPE;
    next_grid[idx].mass = get_mass(SMOKE_TYPE, seed);
    next_grid[idx].velocity = V_STATIONARY;
    return;
  }

  if (bot_valid && IS_JET_FUEL(grid[idx_bot]) && get_rand(seed) % 100 < 100) {
    next_grid[idx].type = FIRE_TYPE;
    next_grid[idx].mass = SCALE_FLOAT(get_rand_float(seed), 0.7f, 4.0f);
    next_grid[idx].velocity = V_STATIONARY;
    return;
  }

  if (bot_valid && !IS_FLUID(grid[idx_bot]) && get_rand(seed) % 10 < 5) {
    next_grid[idx].type = FIRE_TYPE;
    next_grid[idx].mass = SCALE_FLOAT(get_rand_float(seed), 0.5f, 4.0f);
    next_grid[idx].velocity = V_STATIONARY;
    return;
  }

  if (bot_valid && (IS_AIR(grid[idx_bot]) || IS_SMOKE(grid[idx_bot]))) {
    next_grid[idx_bot].type = JET_FUEL_TYPE;
    next_grid[idx_bot].mass = grid[idx].mass;
    next_grid[idx_bot].velocity = grid[idx].velocity;
    next_grid[idx_bot].updated = false;

    next_grid[idx].type = AIR_TYPE;
    next_grid[idx].mass = AIR_MASS;
    next_grid[idx].velocity = V_STATIONARY;

    grid[idx_bot].updated = true;
    return;
  }

  const int direction = get_rand(seed) % 2 == 0 ? -1 : 1;
  const bool dir_valid = direction == -1 ? top_valid : bot_valid;
  const int next_idx = GET_INDEX(row + direction, col, width, height);
  if (dir_valid && (IS_AIR(grid[next_idx]) || IS_SMOKE(grid[next_idx]))) {
    next_grid[next_idx].type = JET_FUEL_TYPE;
    next_grid[next_idx].mass = grid[idx].mass;
    next_grid[next_idx].velocity = grid[idx].velocity;
    next_grid[next_idx].updated = false;

    next_grid[idx].type = AIR_TYPE;
    next_grid[idx].mass = AIR_MASS;
    next_grid[idx].velocity = V_STATIONARY;

    grid[next_idx].updated = true;
    return;
  }

  next_grid[idx].type = JET_FUEL_TYPE;
  next_grid[idx].mass = grid[idx].mass;
  next_grid[idx].velocity = grid[idx].velocity;
}

STEP_IMPL(stone_step) {}

float fluid_get_stable_state(const float total_mass) {
  const float max_mass = 1.0f;
  const float max_compress = 0.01f;

  if (total_mass <= max_mass) {
    return 1;
  } else if (total_mass < 2 * max_mass + max_compress) {
    return (max_mass * max_mass + total_mass * max_compress) /
           (max_mass + max_compress);
  } else {
    return (total_mass + max_compress) / 2;
  }
}

STEP_IMPL(oil_step) {
  GEN_STEP_IMPL_HEADER();

  const float min_mass = 0.000f;
  const float max_speed = 1.00f;
  const float min_flow = 0.005f;
  const float dampen = 0.75f;
  const int horizontal_reach = 2;
  const int bot_reach = 2;

  if (next_grid[idx].updated)
    next_grid[idx].mass = grid[idx].mass;

  // mark cell calculated
  next_grid[idx].updated = false;
  next_grid[idx].type = OIL_TYPE;
  grid[idx].updated = true;

  float remaining_mass = next_grid[idx].mass;
  if (remaining_mass <= min_mass) {
    next_grid[idx].type = AIR_TYPE;
    next_grid[idx].mass = AIR_MASS;
    return;
  }

  // downward flow
  if (bot_valid
          && IS_FLUID(grid[idx_bot])
          && !IS_WATER(grid[idx_bot])
          && !IS_WATER(next_grid[idx_bot])
          && !IS_SAND(next_grid[idx_bot])) {
    next_grid[idx_bot].type = OIL_TYPE;
    next_grid[idx_bot].updated = false;

    float flow =
        fluid_get_stable_state(remaining_mass + next_grid[idx_bot].mass) -
        next_grid[idx_bot].mass;
    flow *= flow > min_flow ? dampen : 1.f;
    flow = FCLAMP(flow, 0, fmin(max_speed, remaining_mass));

    remaining_mass -= flow;
    next_grid[idx].mass -= flow;
    next_grid[idx_bot].mass += flow;

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }
  }

  // downward horizontal flow
  {
    for (int left = (int)col - 1, down = 1;
         left >= ((int)col - bot_reach) && left >= 0 && (row + down) < height;
         left -= 1, down += 1) {
      const uint next_idx = GET_INDEX(row + down, left, width, height);
      if (!IS_FLUID(grid[next_idx])
              || IS_WATER(grid[next_idx])
              || IS_SAND(next_grid[next_idx]))
        break;

      float flow = (next_grid[idx].mass - next_grid[next_idx].mass);
      flow *= flow > min_flow ? dampen : 1.f;
      flow = FCLAMP(flow, 0, next_grid[idx].mass);

      remaining_mass -= flow;

      // if the block below is sand, 30% chance it'll get displaced to the
      // bottom right.
      if (IS_SAND(next_grid[idx_bot]) && remaining_mass > 0.01f &&
          get_rand(seed) % 10 == 0) {
        // new sand block to the bottom left.
        next_grid[next_idx].type = SAND_TYPE;
        next_grid[next_idx].mass = SAND_MASS;
        next_grid[next_idx].updated = false;

        // new oil block below.
        next_grid[idx].mass -= flow;
        next_grid[idx_bot].type = OIL_TYPE;
        next_grid[idx_bot].mass = flow;
        next_grid[idx_bot].updated = false;
        break;
      } else {
        next_grid[next_idx].type = OIL_TYPE;
        next_grid[next_idx].updated = false;
        next_grid[next_idx].mass += flow;
        next_grid[idx].mass -= flow;
      }
    }

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }

    // to the right
    for (int right = (int)col + 1, down = 1;
         right <= ((int)col + bot_reach) && right < width &&
         (row + down) < height;
         right += 1, down += 1) {
      const uint next_idx = GET_INDEX(row + down, right, width, height);
      if (!IS_FLUID(next_grid[next_idx])
              || IS_WATER(grid[next_idx])
              || IS_SAND(next_grid[next_idx]))
        break;

      float flow = (next_grid[idx].mass - next_grid[next_idx].mass);
      flow *= flow > min_flow ? dampen : 1.f;
      flow = FCLAMP(flow, 0, next_grid[idx].mass);

      remaining_mass -= flow;

      // if the block below is sand, 30% chance it'll get displaced to the
      // bottom left
      if (IS_SAND(next_grid[idx_bot]) && remaining_mass > 0.01f &&
          get_rand(seed) % 10 == 0) {
        // new sand block to the bottom left
        next_grid[next_idx].type = SAND_TYPE;
        next_grid[next_idx].mass = SAND_MASS;
        next_grid[next_idx].updated = false;

        // new water block below
        next_grid[idx].mass -= flow;
        next_grid[idx_bot].type = OIL_TYPE;
        next_grid[idx_bot].mass = flow;
        next_grid[idx_bot].updated = false;
        break;

      } else {
        next_grid[next_idx].type = OIL_TYPE;
        next_grid[next_idx].updated = false;
        next_grid[next_idx].mass += flow;
        next_grid[idx].mass -= flow;
      }
    }

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }
  }

  // horizontal flow
  {
    int left = (int)col - 1;
    int right = (int)col + 1;
    float mass_left = 0.f;
    float mass_right = 0.f;

    // find left bound
    for (; left >= max((int)col - horizontal_reach, 0); left -= 1) {
      const uint idx = GET_INDEX(row, left, width, height);
      if (!IS_FLUID(next_grid[idx])
              || IS_WATER(next_grid[idx])
              || IS_SAND(next_grid[idx]))
        break;

      mass_left += next_grid[idx].mass;
    }

    // find right bound
    for (; right <= min(col + horizontal_reach, width - 1); right += 1) {
      const uint idx = GET_INDEX(row, right, width, height);
      if (!IS_FLUID(next_grid[idx])
              || IS_WATER(next_grid[idx])
              || IS_SAND(next_grid[idx]))
        break;

      mass_right += next_grid[idx].mass;
    }

    // correct
    left++;
    right--;

    float mean_mass = (mass_left + mass_right + next_grid[idx].mass);
    mean_mass /= (right - left + 1);

#ifdef DEBUG
    if (mean_mass < 0.0f) {
      printf("%f-%f-%f-%f\n", mean_mass, mass_left, mass_right,
             next_grid[idx].mass);

    } else {
#endif

#define __assign_mean_mass_loop__(init, cond, update)                          \
  {                                                                            \
    for (init; cond; update) {                                                 \
      const uint next_idx = GET_INDEX(row, j, width, height);                  \
      next_grid[next_idx].type = OIL_TYPE;                                   \
      next_grid[next_idx].mass += mean_mass;                                   \
      next_grid[next_idx].mass /= 2;                                           \
      next_grid[next_idx].updated = false;                                     \
    }                                                                          \
  }

      // prefer direction with less mass
      if (mass_right > mass_left) {
        __assign_mean_mass_loop__(int j = col - 1, j >= left, j -= 1);
        __assign_mean_mass_loop__(int j = col + 1, j <= right, j += 1);
      } else {
        __assign_mean_mass_loop__(int j = col + 1, j <= right, j += 1);
        __assign_mean_mass_loop__(int j = col - 1, j >= left, j -= 1);
      }

      next_grid[idx].mass = (remaining_mass + mean_mass) / 2;
      remaining_mass = next_grid[idx].mass;

      if (remaining_mass <= min_mass) {
        next_grid[idx].type = AIR_TYPE;
        next_grid[idx].mass = AIR_MASS;
        return;
      }

#ifdef DEBUG
    }
#endif
  }

  // upward flow
  if (top_valid
          && IS_FLUID(next_grid[idx_top])
          && !IS_WATER(next_grid[idx_top])
          && !IS_SAND(next_grid[idx_top])) {
    next_grid[idx_top].type = OIL_TYPE;
    next_grid[idx_top].updated = false; // other cells updated by this

    float flow = remaining_mass - fluid_get_stable_state(
                                      remaining_mass + next_grid[idx_top].mass);
    flow *= flow > min_flow ? 0.5f : 1;
    flow = FCLAMP(flow, 0.0f, fmin(max_speed, remaining_mass));

    remaining_mass -= flow;
    next_grid[idx].mass -= flow;
    next_grid[idx_top].mass += flow;

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }
  }

  // set remaining mass to this cell
  next_grid[idx].mass = remaining_mass;

}
STEP_IMPL(jello_step) {}

//  water
STEP_IMPL(water_step) {
  GEN_STEP_IMPL_HEADER();

  const float min_mass = 0.000f;
  const float max_speed = 1.00f;
  const float min_flow = 0.005f;
  const float dampen = 0.75f;
  const int horizontal_reach = 2;
  const int bot_reach = 2;

  if (next_grid[idx].updated)
    next_grid[idx].mass = grid[idx].mass;

  // mark cell calculated
  next_grid[idx].updated = false;
  next_grid[idx].type = WATER_TYPE;
  grid[idx].updated = true;

  float remaining_mass = next_grid[idx].mass;
  if (remaining_mass <= min_mass) {
    next_grid[idx].type = AIR_TYPE;
    next_grid[idx].mass = AIR_MASS;
    return;
  }

  // downward flow
  if (bot_valid
          && IS_FLUID(grid[idx_bot])
          && !IS_OIL(grid[idx_bot])
          && !IS_SAND(next_grid[idx_bot])) {
    next_grid[idx_bot].type = WATER_TYPE;
    next_grid[idx_bot].updated = false;

    float flow =
        fluid_get_stable_state(remaining_mass + next_grid[idx_bot].mass) -
        next_grid[idx_bot].mass;
    flow *= flow > min_flow ? dampen : 1.f;
    flow = FCLAMP(flow, 0, fmin(max_speed, remaining_mass));

    remaining_mass -= flow;
    next_grid[idx].mass -= flow;
    next_grid[idx_bot].mass += flow;

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }
  }

  // downward horizontal flow
  {
    for (int left = (int)col - 1, down = 1;
         left >= ((int)col - bot_reach) && left >= 0 && (row + down) < height;
         left -= 1, down += 1) {
      const uint next_idx = GET_INDEX(row + down, left, width, height);
      if (!IS_FLUID(grid[next_idx])
              || IS_OIL(grid[next_idx])
              || IS_SAND(next_grid[next_idx]))
        break;

      float flow = (next_grid[idx].mass - next_grid[next_idx].mass);
      flow *= flow > min_flow ? dampen : 1.f;
      flow = FCLAMP(flow, 0, next_grid[idx].mass);

      remaining_mass -= flow;

      // if the block below is sand, 2% chance it'll get displaced to the
      // bottom right.
      if (IS_SAND(next_grid[idx_bot]) && remaining_mass > 0.005f &&
          get_rand(seed) % 100 < 3) {
        // new sand block to the bottom left.
        next_grid[next_idx].type = SAND_TYPE;
        next_grid[next_idx].mass = SAND_MASS;
        next_grid[next_idx].updated = false;

        // new water block below.
        next_grid[idx].mass -= flow;
        next_grid[idx_bot].type = WATER_TYPE;
        next_grid[idx_bot].mass = flow;
        next_grid[idx_bot].updated = false;
        break;
      } else {
        next_grid[next_idx].type = WATER_TYPE;
        next_grid[next_idx].updated = false;
        next_grid[next_idx].mass += flow;
        next_grid[idx].mass -= flow;
      }
    }

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }

    // to the right
    for (int right = (int)col + 1, down = 1;
         right <= ((int)col + bot_reach) && right < width &&
         (row + down) < height;
         right += 1, down += 1) {
      const uint next_idx = GET_INDEX(row + down, right, width, height);
      if (!IS_FLUID(next_grid[next_idx])
              || IS_OIL(grid[next_idx])
              || IS_SAND(next_grid[next_idx]))
        break;

      float flow = (next_grid[idx].mass - next_grid[next_idx].mass);
      flow *= flow > min_flow ? dampen : 1.f;
      flow = FCLAMP(flow, 0, next_grid[idx].mass);

      remaining_mass -= flow;

      // if the block below is sand, 2% chance it'll get displaced to the
      // bottom left
      if (IS_SAND(next_grid[idx_bot]) && remaining_mass > 0.005f &&
          get_rand(seed) % 100 < 2) {
        // new sand block to the bottom left
        next_grid[next_idx].type = SAND_TYPE;
        next_grid[next_idx].mass = SAND_MASS;
        next_grid[next_idx].updated = false;

        // new water block below
        next_grid[idx].mass -= flow;
        next_grid[idx_bot].type = WATER_TYPE;
        next_grid[idx_bot].mass = flow;
        next_grid[idx_bot].updated = false;
        break;

      } else {
        next_grid[next_idx].type = WATER_TYPE;
        next_grid[next_idx].updated = false;
        next_grid[next_idx].mass += flow;
        next_grid[idx].mass -= flow;
      }
    }

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }
  }

  // horizontal flow
  {
    int left = (int)col - 1;
    int right = (int)col + 1;
    float mass_left = 0.f;
    float mass_right = 0.f;

    // find left bound
    for (; left >= max((int)col - horizontal_reach, 0); left -= 1) {
      const uint idx = GET_INDEX(row, left, width, height);
      if (!IS_FLUID(next_grid[idx])
              || IS_OIL(next_grid[idx])
              || IS_SAND(next_grid[idx]))
        break;

      mass_left += next_grid[idx].mass;
    }

    // find right bound
    for (; right <= min(col + horizontal_reach, width - 1); right += 1) {
      const uint idx = GET_INDEX(row, right, width, height);
      if (!IS_FLUID(next_grid[idx])
              || IS_OIL(next_grid[idx])
              || IS_SAND(next_grid[idx]))
        break;

      mass_right += next_grid[idx].mass;
    }

    // correct
    left++;
    right--;

    float mean_mass = (mass_left + mass_right + next_grid[idx].mass);
    mean_mass /= (right - left + 1);

#ifdef DEBUG
    if (mean_mass < 0.0f) {
      printf("%f-%f-%f-%f\n", mean_mass, mass_left, mass_right,
             next_grid[idx].mass);

    } else {
#endif

#define __assign_mean_mass_loop__(init, cond, update)                          \
  {                                                                            \
    for (init; cond; update) {                                                 \
      const uint next_idx = GET_INDEX(row, j, width, height);                  \
      next_grid[next_idx].type = WATER_TYPE;                                   \
      next_grid[next_idx].mass += mean_mass;                                   \
      next_grid[next_idx].mass /= 2;                                           \
      next_grid[next_idx].updated = false;                                     \
    }                                                                          \
  }

      // prefer direction with less mass
      if (mass_right > mass_left) {
        __assign_mean_mass_loop__(int j = col - 1, j >= left, j -= 1);
        __assign_mean_mass_loop__(int j = col + 1, j <= right, j += 1);
      } else {
        __assign_mean_mass_loop__(int j = col + 1, j <= right, j += 1);
        __assign_mean_mass_loop__(int j = col - 1, j >= left, j -= 1);
      }

      next_grid[idx].mass = (remaining_mass + mean_mass) / 2;
      remaining_mass = next_grid[idx].mass;

      if (remaining_mass <= min_mass) {
        next_grid[idx].type = AIR_TYPE;
        next_grid[idx].mass = AIR_MASS;
        return;
      }

#ifdef DEBUG
    }
#endif
  }

  // upward flow
  if (top_valid
          && IS_FLUID(next_grid[idx_top])
          && !IS_OIL(next_grid[idx_top])
          && !IS_SAND(next_grid[idx_top])) {
    next_grid[idx_top].type = WATER_TYPE;
    next_grid[idx_top].updated = false; // other cells updated by this

    float flow = remaining_mass - fluid_get_stable_state(
                                      remaining_mass + next_grid[idx_top].mass);
    flow *= flow > min_flow ? 0.5f : 1;
    flow = FCLAMP(flow, 0.0f, fmin(max_speed, remaining_mass));

    remaining_mass -= flow;
    next_grid[idx].mass -= flow;
    next_grid[idx_top].mass += flow;

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }
  }

  // set remaining mass to this cell
  next_grid[idx].mass = remaining_mass;
}

//  sand
STEP_IMPL(sand_step) {
  GEN_STEP_IMPL_HEADER();

#define __move_sand__(idx_next)                                                \
  {                                                                            \
    next_grid[idx].type = AIR_TYPE;                                            \
    next_grid[idx_next].type = SAND_TYPE;                                      \
    next_grid[idx].mass = AIR_MASS;                                            \
    next_grid[idx_next].mass = SAND_MASS;                                      \
    next_grid[idx].velocity = V_STATIONARY;                                    \
    next_grid[idx_next].velocity = V_STATIONARY;                               \
    next_grid[idx].updated = false;                                            \
    next_grid[idx_next].updated = false;                                       \
    grid[idx].updated = true;                                                  \
    grid[idx_next].updated = true;                                             \
  }

  if (!bot_valid)
    return;

  // move down if possible
  if (IS_FLUID(grid[idx_bot])) {
    int replacement_type = AIR_TYPE;
    float replacement_mass = AIR_MASS;

    // 45% chance water will be pushed above by sand
    // 50% change water will eat sand away
    if (get_rand(seed) % 100 < 45) {
      replacement_mass = next_grid[idx_bot].mass;
      replacement_type = next_grid[idx_bot].type;
    }

    next_grid[idx].type = replacement_type;
    next_grid[idx_bot].type = SAND_TYPE;

    next_grid[idx].mass = replacement_mass;
    next_grid[idx_bot].mass = SAND_MASS;

    next_grid[idx].velocity = V_STATIONARY;
    next_grid[idx_bot].velocity = V_STATIONARY;

    next_grid[idx].updated = false;
    next_grid[idx_bot].updated = false;

    // mark this cell updated
    grid[idx].updated = true;
    grid[idx_bot].updated = true;
  }

  // move down left/right if possible with uniform probability
  // TODO(vir): improve random number generation for this case
  else if (get_rand(seed) % 2 == 0) {

    // prefer left
    if (left_valid && IS_FLUID(grid[idx_bot_left]) &&
        !grid[idx_bot_left].updated) {
      __move_sand__(idx_bot_left);
    }

    else if (right_valid && IS_FLUID(grid[idx_bot_right]) &&
             !grid[idx_bot_right].updated) {
      __move_sand__(idx_bot_right);
    }

  } else {

    // prefer right
    if (right_valid && IS_FLUID(grid[idx_bot_right]) &&
        !grid[idx_bot_right].updated) {
      __move_sand__(idx_bot_right);
    }

    else if (left_valid && IS_FLUID(grid[idx_bot_left]) &&
             !grid[idx_bot_left].updated) {
      __move_sand__(idx_bot_left);
    }
  }

  return;
}

//  If water is above oil, swap.
STEP_IMPL(water_oil_step) {
  GEN_STEP_IMPL_HEADER();

  if (bot_valid && IS_WATER(grid[idx]) && IS_OIL(grid[idx_bot])) {
    float water_mass = grid[idx].mass;
    // Oil comes up.
    next_grid[idx].type = OIL_TYPE;
    next_grid[idx].mass = grid[idx_bot].mass;
    // Water goes down.
    next_grid[idx_bot].type = WATER_TYPE;
    next_grid[idx_bot].mass = water_mass;
  }

  return;
}

#endif
