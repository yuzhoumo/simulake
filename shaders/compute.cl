// vim: ft=cpp :

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
#define IS_AIR(x) (x.type == AIR_TYPE)
#define IS_WATER(x) (x.type == WATER_TYPE)
#define IS_FLAMMABLE(x)                                                        \
  (x.type >= AIR_TYPE && (x.type == OIL_TYPE || x.type == SAND_TYPE))

#define FCLAMP(x, l, h) (fmax((float)l, fmin((float)x, (float)h)))

// #define ROW_MAJOR(row, col, width, height) (((row) * (width)) + (col))

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
// TODO(vir): benchmark
typedef struct __attribute__((packed, aligned(8))) {
  char type;
  float mass;

  bool updated;
} grid_t;

float get_stable_state_b(const float total_mass) {
  const float max_mass = 0.8f;
  const float max_compress = 0.02f;

  if (total_mass <= 1) {
    return 1;
  } else if (total_mass < 2 * max_mass + max_compress) {
    return (max_mass * max_mass + total_mass * max_compress) /
           (max_mass + max_compress);
  } else {
    return (total_mass + max_compress) / 2;
  }
}

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

  // horizontal bar
  if (abs(row - (height / 2) - 5) < 5 && width - col > 100 && col > 100) {
    grid[idx].type = STONE_TYPE;
    next_grid[idx].type = STONE_TYPE;

    grid[idx].mass = STONE_MASS;
    next_grid[idx].mass = STONE_MASS;
  }
}

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

  // sand step
  if (type == SAND_TYPE && bot_valid) { // cant fall below ground
    // printf("%d-%d-%d\n", grid > next_grid ? 0 : 1, idx, idx_bot);

    if (VACANT(grid[idx_bot]) && !grid[idx_bot].updated) {
      next_grid[idx].type = grid[idx_bot].type;
      next_grid[idx_bot].type = SAND_TYPE;

      next_grid[idx].mass = grid[idx_bot].mass;
      next_grid[idx_bot].mass = SAND_MASS;

      next_grid[idx].updated = false;
      next_grid[idx_bot].updated = false;

      // mark this cell updated
      grid[idx].updated = true;
      grid[idx_bot].updated = true;
    }

    else if (left_valid && VACANT(grid[idx_bot_left]) &&
             !grid[idx_bot_left].updated) {
      next_grid[idx].type = grid[idx_bot_left].type;
      next_grid[idx_bot_left].type = SAND_TYPE;

      next_grid[idx].mass = grid[idx_bot_left].mass;
      next_grid[idx_bot_left].mass = SAND_MASS;

      next_grid[idx].updated = false;
      next_grid[idx_bot_left].updated = false;

      // mark this cell updated
      grid[idx].updated = true;
      grid[idx_bot_left].updated = true;
    }

    else if (right_valid && VACANT(grid[idx_bot_right]) &&
             !grid[idx_bot_right].updated) {
      next_grid[idx].type = grid[idx_bot_right].type;
      next_grid[idx_bot_right].type = SAND_TYPE;

      next_grid[idx].mass = grid[idx_bot_right].mass;
      next_grid[idx_bot_right].mass = SAND_MASS;

      next_grid[idx].updated = false;
      next_grid[idx_bot_right].updated = false;

      // mark this cell updated
      grid[idx_bot_right].updated = true;
      grid[idx_bot_right].updated = true;
    }

    return;
  }

  // water step
  // else if (type == WATER_TYPE && !grid[idx].updated) {
  else if (type == WATER_TYPE) {
    // printf("row: %d, col: %d\n", row, col);

    const float min_mass = 0.0001f;
    const float max_speed = 0.7f;
    const float min_flow = 0.01f;
    const int horizontal_reach = 2;

    float flow = 0;
    float remaining_mass = next_grid[idx].mass;

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      next_grid[idx].updated = false;
      grid[idx].updated = true;
      return;
    }

    // try to flow down
    if (IS_FLUID(grid[idx_bot]) && bot_valid) {
      next_grid[idx_bot].type = WATER_TYPE;
      next_grid[idx_bot].updated = false;
      grid[idx].updated = true;
      next_grid[idx].updated = false; // other cells updated by this call

      // calculate mass flow
      flow = get_stable_state_b(remaining_mass + next_grid[idx_bot].mass) -
             next_grid[idx_bot].mass;
      flow *= flow > min_flow ? 0.5f : 1; // smoothen flow
      flow = FCLAMP(flow, 0, fmin(max_speed, remaining_mass));

      remaining_mass -= flow;
      next_grid[idx].mass -= flow;
      next_grid[idx_bot].mass += flow;
    }

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].mass = AIR_MASS;
      next_grid[idx].updated = false;
      grid[idx].updated = true;
      return;
    }

    {
      int left = col;
      int right = col;
      float mass_left = 0.f;
      float mass_right = 0.f;

      // find left bound
      for (; left >= max((int)col - horizontal_reach, 0); left -= 1) {
        const uint idx = GET_INDEX(row, left, width, height);
        if (!IS_FLUID(grid[idx]))
          break;

        mass_left += next_grid[idx].mass;
      }
      left++;

      // find right bound
      for (; right <= min(col + horizontal_reach, width - 1); right += 1) {
        const uint idx = GET_INDEX(row, right, width, height);
        if (!IS_FLUID(grid[idx]))
          break;

        mass_right += next_grid[idx].mass;
      }
      right--;

      // mass_left /= (col - left + 1);
      // mass_right /= (right - col + 1);

      // Find mean mass.
      float mean_mass = .0f;
      for (int j = left; j <= right; ++j)
        mean_mass += next_grid[GET_INDEX(row, j, width, height)].mass;
      mean_mass /= (right - left + 1);

      // equalize-ish
      // prefer direction of less mass
      if (mass_right > mass_left) {
        for (int j = left; j <= right; j += 1) {
          uint idx = GET_INDEX(row, j, width, height);
          next_grid[idx].type = WATER_TYPE;
          // grid._next_mass[x][j] = mean_mass;
          next_grid[idx].mass += mean_mass;
          next_grid[idx].mass /= 2;
          next_grid[idx].updated = true;
        }

      } else {
        for (int j = right; j >= left; j -= 1) {
          uint idx = GET_INDEX(row, j, width, height);
          next_grid[idx].type = WATER_TYPE;
          // grid._next_mass[x][j] = mean_mass;
          next_grid[idx].mass += mean_mass;
          next_grid[idx].mass /= 2;
          next_grid[idx].updated = true;
        }
      }

      remaining_mass = next_grid[idx].mass;
    }

    if (remaining_mass <= min_mass) {
      next_grid[idx].type = AIR_TYPE;
      next_grid[idx].updated = false;
      grid[idx].updated = true;
      return;
    }

    // if compressed, flow up
    if (IS_FLUID(grid[idx_top]) && top_valid) {
      next_grid[idx].type = WATER_TYPE;
      next_grid[idx].updated = false;
      grid[idx].updated = true;
      next_grid[idx_top].updated = false; // other cells updated by this cell

      flow = remaining_mass -
             get_stable_state_b(remaining_mass + next_grid[idx_top].mass);
      flow *= flow > min_flow ? 0.5f : 1;
      flow = FCLAMP(flow, 0, fmin(max_speed, remaining_mass));

      remaining_mass -= flow;
      next_grid[idx].mass -= flow;
      next_grid[idx_top].mass += flow;
    }
  }
}

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
  const int2 out_coord = {width - col, height - row};
  write_imagef(texture, out_coord, out_color);
}

__kernel void spawn_cells(__global grid_t *grid, __global grid_t *next_grid,
                          const float2 mouse, const float paint_radius,
                          const uint target, const uint2 dims,
                          const uint cell_size) {
  const uint col = get_global_id(0); // <= local grid size (cols)
  const uint row = get_global_id(1); // <= local grid size (rows)

  const uint width = dims[0];
  const uint height = dims[1];
  const uint screen_col = width - col - 1;

  const float2 mouse_norm = mouse / cell_size;

  const uint idx = GET_INDEX(row, screen_col, width, height);
  const float d =
      sqrt(pow(col - (mouse_norm[0]), 2) + pow(row - mouse_norm[1], 2));

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
