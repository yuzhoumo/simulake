// vim: ft=cpp : foldmarker={{{,}}} :

// TODO(vir): benchmark?
#define USE_ROWMAJOR true

// clang-format off
#define   NONE_TYPE    0
#define   AIR_TYPE     1
#define   SMOKE_TYPE   2
#define   FIRE_TYPE    3
#define   WATER_TYPE   4
#define   OIL_TYPE     5
#define   SAND_TYPE    6
#define   JELLO_TYPE   7
#define   STONE_TYPE   8
// clang-format on

#define FALLS_DOWN(x) (x.type > WATER_TYPE)
#define VACANT(x) (x.type == AIR_TYPE)

#define IS_FLUID(x) (x.type >= AIR_TYPE && x.type <= OIL_TYPE)
#define IS_LIQUID(x) (x.type >= WATER_TYPE && x.type <= OIL_TYPE)
#define IS_AIR(x) (x.type == AIR_TYPE)
#define IS_WATER(x) (x.type == WATER_TYPE)
#define IS_FLAMMABLE(x)                                                        \
  (x.type >= AIR_TYPE && (x.type == OIL_TYPE || x.type == SAND_TYPE))

#define FCLAMP(x, l, h) (fmax((float)l, fmin((float)x, (float)h)))

#if USE_ROWMAJOR
#define GET_INDEX(row, col, width, height) (((col) * (height)) + (row))
#else
#define GET_INDEX(row, col, width, height) (((row) * (width)) + (col))
#endif

// clang-format off
#define   AIR_MASS     0.0f
#define   SMOKE_MASS   0.2f
#define   FIRE_MASS    0.3f
#define   WATER_MASS   1.0f
#define   OIL_MASS     0.8f
#define   SAND_MASS    1.5f
#define   JELLO_MASS   1.7f
#define   STONE_MASS   3.0f
// clang-format on

#define ASSERT_TRUE(x)                                                         \
  {                                                                            \
    if (!(x))                                                                  \
      printf("assert_fail: (%s) was false @ %d\n", #x, __LINE__);              \
  }

#define ASSERT_FALSE(x)                                                        \
  {                                                                            \
    if ((x))                                                                   \
      printf("assert_fail: (%s) was true @ %d\n", #x, __LINE__);               \
  }

// get a random number (0, UINT_MAX) using xor shift
inline uint get_rand(const uint row, const uint col) {
  const uint seed = 1337 + row;
  const uint t = seed ^ (seed << 11);
  return (7331 + col) ^ ((7331 + col) >> 19) ^ (t ^ (t >> 8));
}

inline float get_mass(const uint type) {
  switch (type) {
  case SMOKE_TYPE:
    return SMOKE_MASS;
    break;
  case FIRE_TYPE:
    return FIRE_MASS;
    break;
  case WATER_TYPE:
    return WATER_MASS;
    break;
  case OIL_TYPE:
    return OIL_MASS;
    break;
  case SAND_TYPE:
    return SAND_MASS;
    break;
  case JELLO_TYPE:
    return JELLO_MASS;
    break;

  case AIR_TYPE:
  default:
    return AIR_MASS;
  }
}

// cell attributes
// TODO(vir): benchmark?
typedef struct __attribute__((packed, aligned(8))) {
  char type;
  float mass;

  bool updated;
} grid_t;

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

// {{{ initialize kernel
__kernel void initialize(__global grid_t *grid, __global grid_t *next_grid,
                         uint2 dims) {
  const uint size = get_global_size(0); // full grid size (rows)
  const uint col = get_global_id(0);
  const uint row = get_global_id(1);
  const uint width = dims[0];
  const uint height = dims[1];

  const unsigned int idx = GET_INDEX(row, col, width, height);
  // printf("%d-%d-%d\n", row, col, idx);

  grid[idx].type = AIR_TYPE;
  next_grid[idx].type = AIR_TYPE;

  grid[idx].mass = AIR_MASS;
  next_grid[idx].mass = AIR_MASS;

  grid[idx].updated = false;
  next_grid[idx].updated = false;

  // // draw box with water
  // if (abs(row - (height / 2) - 5) < 5 &&
  //     abs(col - width / 4 - 5) < width / 2 + 5) {
  //   grid[idx].type = STONE_TYPE;
  //   next_grid[idx].type = STONE_TYPE;

  //   grid[idx].mass = STONE_MASS;
  //   next_grid[idx].mass = STONE_MASS;
  // }

  // else if ((abs(col - (width * 1 / 4) - 5) < 5 ||
  //           abs(col - (width * 3 / 4) - 5) < 5) &&
  //          abs(row - height / 2 + 80) < 100) {
  //   grid[idx].type = STONE_TYPE;
  //   next_grid[idx].type = STONE_TYPE;

  //   grid[idx].mass = STONE_MASS;
  //   next_grid[idx].mass = STONE_MASS;
  // }

  // else if (get_rand(row, col) % 7 == 0) {
  //   grid[idx].type = WATER_TYPE;
  //   next_grid[idx].type = WATER_TYPE;

  //   grid[idx].mass = WATER_MASS;
  //   next_grid[idx].mass = WATER_MASS;
  // }
}
// }}}

// {{{ random init kernel
__kernel void random_init(__global grid_t *grid,
                          const __global grid_t *next_grid, const uint2 dims) {
  const uint col = get_global_id(0);
  const uint row = get_global_id(1);

  const uint width = dims[0];
  const uint height = dims[1];
  const uint idx = GET_INDEX(row, col, width, height);
  const uint rand = get_rand(row, col);

  if (rand % 20) {
    grid[idx].type = SAND_TYPE;
    grid[idx].mass = SAND_TYPE;
    grid[idx].updated = false;
  }
}
// }}}

//  simulate kernel
__kernel void simulate(__global grid_t *grid, __global grid_t *next_grid,
                       const uint2 dims) {
  const uint col = get_global_id(0); // <= local grid size (cols)
  const uint row = get_global_id(1); // <= local grid size (rows)

  const uint width = dims[0];
  const uint height = dims[1];
  const uint num_cells = width * height;

  const bool top_valid = (row - 1) >= 0;
  const bool bot_valid = (row + 1) < height;
  const bool left_valid = ((long)col - 1) > 0;
  const bool right_valid = (col + 1) < width;

  // clang-format off
  const uint idx_top_left  = GET_INDEX(row - 1, col - 1, width, height);
  const uint idx_top       = GET_INDEX(row - 1, col + 0, width, height);
  const uint idx_top_right = GET_INDEX(row - 1, col + 1, width, height);
  const uint idx_left      = GET_INDEX(row + 0, col - 1, width, height);
  const uint idx           = GET_INDEX(row + 0, col + 0, width, height);
  const uint idx_right     = GET_INDEX(row + 0, col + 1, width, height);
  const uint idx_bot_left  = GET_INDEX(row + 1, col - 1, width, height);
  const uint idx_bot       = GET_INDEX(row + 1, col + 0, width, height);
  const uint idx_bot_right = GET_INDEX(row + 1, col + 1, width, height);
  // clang-format on

  const uint type = (uint)grid[idx].type; // scale up from std::uint8_t

  // {{{ sand step
  if (type == SAND_TYPE && bot_valid) { // cant fall below ground
    // printf("%d-%d-%d\n", grid > next_grid ? 0 : 1, idx, idx_bot);
    const uint rand = get_rand(row, col);

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

    if (IS_FLUID(grid[idx_bot]) && !grid[idx_bot].updated) {
      __move_sand__(idx_bot);
    }

    else if (rand % 2 == 0) {

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

  // {{{ water step
  else if (type == WATER_TYPE && !grid[idx].updated) {
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
    grid[idx].updated = true;
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

      float flow =
          remaining_mass -
          water_get_stable_state(remaining_mass + next_grid[idx_top].mass);
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

    next_grid[idx].mass = remaining_mass;
  }
  // }}}
}

// {{{ render texture kernel
__kernel void render_texture(__write_only image2d_t texture,
                             __global const grid_t *grid,
                             __global const grid_t *next_grid, const uint2 dims,
                             const uint cell_size) {
  const uint col = get_global_id(0); // <= local grid size (cols)
  const uint row = get_global_id(1); // <= local grid size (rows)

  const uint width = dims[0];
  const uint height = dims[1];

  const uint idx = GET_INDEX(row, col, width, height);
  const uint type = (int)grid[idx].type; // scale up from std::uint8_t

  // write texture
  // attributes go here
  const float4 out_color = {type, grid[idx].mass, 0.f, 0.f};
  const int2 out_coord = {width - col - 1, height - row - 1};
  write_imagef(texture, out_coord, out_color);
}
// }}}

// {{{ spawn cells kernel
__kernel void spawn_cells(__global grid_t *grid, __global grid_t *next_grid,
                          const uint2 center, const uint paint_radius,
                          const uint target, const uint2 dims,
                          const uint cell_size) {
  const uint col = get_global_id(0); // <= local grid size (cols)
  const uint row = get_global_id(1); // <= local grid size (rows)

  const uint width = dims.x;
  const uint height = dims.y;
  const uint screen_col = width - col - 1;

  const uint idx = GET_INDEX(row, screen_col, width, height);
  const float d =
      sqrt(pow(col - (float)center.x, 2) + pow(row - (float)center.y, 2));

  if (d <= paint_radius && (VACANT(grid[idx]) || (target == AIR_TYPE))) {
    next_grid[idx].type = target;
    grid[idx].type = target;

    const float mass = get_mass(target);
    next_grid[idx].mass = mass;
    grid[idx].mass = mass;

    next_grid[idx].updated = false;
    grid[idx].updated = false;
  }
}
// }}}
