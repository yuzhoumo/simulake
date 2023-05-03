// vim: ft=cpp :

// base macros and definitions
#include "base.cl"

STEP_IMPL(smoke_step) {}
STEP_IMPL(fire_step) {}
STEP_IMPL(oil_step) {}
STEP_IMPL(jello_step) {}
STEP_IMPL(stone_step) {}

float water_get_stable_state(const float total_mass) {
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

// {{{ water
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
  // grid[idx].updated = true;
  next_grid[idx].updated = false;
  next_grid[idx].type = WATER_TYPE;

  float remaining_mass = next_grid[idx].mass;
  if (remaining_mass <= min_mass) {
    next_grid[idx].type = AIR_TYPE;
    next_grid[idx].mass = AIR_MASS;
    return;
  }

  // downward flow
  if (bot_valid && IS_FLUID(grid[idx_bot])) {
    next_grid[idx_bot].type = WATER_TYPE;
    next_grid[idx_bot].updated = false;

    float flow =
        water_get_stable_state(remaining_mass + next_grid[idx_bot].mass) -
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
      if (!IS_FLUID(grid[next_idx]))
        break;

      next_grid[next_idx].type = WATER_TYPE;
      next_grid[next_idx].updated = false;

      float flow = (next_grid[idx].mass - next_grid[next_idx].mass);
      flow *= flow > min_flow ? dampen : 1.f;
      flow = FCLAMP(flow, 0, next_grid[idx].mass);

      remaining_mass -= flow;
      next_grid[idx].mass -= flow;
      next_grid[next_idx].mass += flow;
    }

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      return;
    }

    for (int right = (int)col + 1, down = 1;
         right <= ((int)col + bot_reach) && right < width &&
         (row + down) < height;
         right += 1, down += 1) {
      const uint next_idx = GET_INDEX(row + down, right, width, height);
      if (!IS_FLUID(grid[next_idx]))
        break;

      next_grid[next_idx].type = WATER_TYPE;
      next_grid[next_idx].updated = false;

      float flow = (next_grid[idx].mass - next_grid[next_idx].mass);
      flow *= flow > min_flow ? dampen : 1.f;
      flow = FCLAMP(flow, 0, next_grid[idx].mass);

      remaining_mass -= flow;
      next_grid[idx].mass -= flow;
      next_grid[next_idx].mass += flow;
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
      if (!IS_FLUID(grid[idx]))
        break;

      mass_left += next_grid[idx].mass;
    }

    // find right bound
    for (; right <= min(col + horizontal_reach, width - 1); right += 1) {
      const uint idx = GET_INDEX(row, right, width, height);
      if (!IS_FLUID(grid[idx]))
        break;

      mass_right += next_grid[idx].mass;
    }

    // correct endpoints
    left++;
    right--;

    float mean_mass = (mass_left + mass_right + next_grid[idx].mass);
    mean_mass /= (right - left + 1);

    // ASSERT_TRUE(mean_mass > 0.0f)

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
  if (top_valid && IS_FLUID(next_grid[idx_top]) && IS_FLUID(grid[idx_top])) {
    next_grid[idx_top].type = WATER_TYPE;
    next_grid[idx_top].updated = false; // other cells updated by this

    float flow = remaining_mass - water_get_stable_state(
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
// }}}

// {{{ sand
STEP_IMPL(sand_step) {
#define __move_sand__(idx_next)                                                \
  {                                                                            \
    next_grid[idx].type = AIR_TYPE;                                            \
    next_grid[idx_next].type = SAND_TYPE;                                      \
    next_grid[idx].mass = AIR_MASS;                                            \
    next_grid[idx_next].mass = SAND_MASS;                                      \
    next_grid[idx].updated = false;                                            \
    next_grid[idx_next].updated = false;                                       \
    grid[idx].updated = true;                                                  \
    grid[idx_next].updated = true;                                             \
  }

  GEN_STEP_IMPL_HEADER();

  // cannot fall below bottom
  if (!bot_valid)
    return;

  // move down if possible
  if (IS_FLUID(grid[idx_bot]) && !grid[idx_bot].updated) {
    __move_sand__(idx_bot);
  }

  // move down left/right if possible with uniform probability
  else if (get_rand(row, col) % 2 == 0) {

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
// }}}
