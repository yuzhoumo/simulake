// vim: ft=cpp :

// clang-format off
#define   NONE_TYPE    0
#define   AIR_TYPE     1
#define   SMOKE_TYPE   2
#define   FIRE_TYPE    3
#define   WATER_TYPE   4
#define   OIL_TYPE     5
#define   SAND_TYPE    6
#define   JELLO_TYPE   7
// clang-format on

#define FALLS_DOWN(x) (x.type > WATER_TYPE)
#define VACANT(x) (x.type == AIR_TYPE)

#define SET_CELL(x, type)                                                      \
  {                                                                            \
    switch (type) {                                                            \
    case SMOKE_TYPE:                                                           \
      SET_SMOKE(x);                                                            \
      break;                                                                   \
    case FIRE_TYPE:                                                            \
      SET_FIRE(x);                                                             \
      break;                                                                   \
    case WATER_TYPE:                                                           \
      SET_WATER(x);                                                            \
      break;                                                                   \
    case OIL_TYPE:                                                             \
      SET_OIL(x);                                                              \
      break;                                                                   \
    case SAND_TYPE:                                                            \
      SET_SAND(x);                                                             \
      break;                                                                   \
    case JELLO_TYPE:                                                           \
      SET_JELLO(x);                                                            \
      break;                                                                   \
    case AIR_TYPE:                                                             \
    default:                                                                   \
      SET_AIR(x);                                                              \
      break;                                                                   \
    };                                                                         \
  }

#define SET_AIR(x)                                                             \
  {                                                                            \
    x.type = AIR_TYPE;                                                         \
    x.mass = 0.0f;                                                             \
  }

#define SET_SMOKE(x)                                                           \
  {                                                                            \
    x.type = SMOKE_TYPE;                                                       \
    x.mass = 1.0f;                                                             \
  }

#define SET_FIRE(x)                                                            \
  {                                                                            \
    x.type = FIRE_TYPE;                                                        \
    x.mass = 1.0f;                                                             \
  }

#define SET_WATER(x)                                                           \
  {                                                                            \
    x.type = WATER_TYPE;                                                       \
    x.mass = 1.0f;                                                             \
  }

#define SET_OIL(x)                                                             \
  {                                                                            \
    x.type = OIL_TYPE;                                                         \
    x.mass = 1.0f;                                                             \
  }

#define SET_SAND(x)                                                            \
  {                                                                            \
    x.type = SAND_TYPE;                                                        \
    x.mass = 1.0f;                                                             \
  }

#define SET_JELLO(x)                                                           \
  {                                                                            \
    x.type = JELLO_TYPE;                                                       \
    x.mass = 1.0f;                                                             \
  }

// cell attributes
// TODO(vir): benchmark
typedef struct __attribute__((packed, aligned(8))) {
  char type;
  float mass;

  bool updated;
} grid_t;

__kernel void initialize(__global grid_t *grid, __global grid_t *next_grid,
                         uint2 dims) {
  const uint size = get_global_size(0); // full grid size (rows)
  const uint col = get_global_id(0);
  const uint row = get_global_id(1);
  const uint width = dims[0];
  const uint height = dims[1];

  const unsigned int idx = row * width + col;
  // printf("%d-%d-%d\n", row, col, idx);

  SET_AIR(grid[idx]);
  SET_AIR(next_grid[idx]);
  grid[idx].updated = false;
  next_grid[idx].updated = false;
}

__kernel void random_init(__global grid_t *grid,
                          const __global grid_t *next_grid, const uint2 dims) {
  const uint col = get_global_id(0);
  const uint row = get_global_id(1);

  const uint width = dims[0];
  const uint height = dims[1];

  // get a random number using xorshift
  const uint seed = 1337 + row;
  const uint t = seed ^ (seed << 11);
  const uint rand = (7331 + col) ^ ((7331 + col) >> 19) ^ (t ^ (t >> 8));

  if (rand % 5) {
    printf("%d\n", col);

    const uint idx = row * width + col;
    SET_SAND(grid[idx]);
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
  const uint idx_top_left  = (row - 1) * width + (col - 1);
  const uint idx_top       = (row - 1) * width + (col + 0);
  const uint idx_top_right = (row - 1) * width + (col + 1);
  const uint idx_left      = (row + 0) * width + (col - 1);
  const uint idx           = (row + 0) * width + (col + 0);
  const uint idx_right     = (row + 0) * width + (col + 1);
  const uint idx_bot_left  = (row + 1) * width + (col - 1);
  const uint idx_bot       = (row + 1) * width + (col + 0);
  const uint idx_bot_right = (row + 1) * width + (col + 1);
  // clang-format on

  const uint type = (uint)grid[idx].type; // scale up from std::uint8_t
  if (type == SAND_TYPE && bot_valid) {   // cant fall below ground
    // printf("%d-%d-%d\n", grid > next_grid ? 0 : 1, idx, idx_bot);

    if (VACANT(grid[idx_bot]) && !grid[idx_bot].updated) {
      SET_AIR(next_grid[idx]);
      SET_SAND(next_grid[idx_bot]);

      next_grid[idx].updated = false;
      next_grid[idx_bot].updated = false;

      // mark this cell updated
      grid[idx].updated = true;
      grid[idx_bot].updated = true;
    }

    else if (left_valid && VACANT(grid[idx_bot_left]) &&
             !grid[idx_bot_left].updated) {
      SET_AIR(next_grid[idx]);
      SET_SAND(next_grid[idx_bot_left]);

      next_grid[idx].updated = false;
      next_grid[idx_bot_left].updated = false;

      // mark this cell updated
      grid[idx].updated = true;
      grid[idx_bot_left].updated = true;
    }

    else if (right_valid && VACANT(grid[idx_bot_right]) &&
             !grid[idx_bot_right].updated) {
      SET_AIR(next_grid[idx]);
      SET_SAND(next_grid[idx_bot_right]);

      next_grid[idx].updated = false;
      next_grid[idx_bot_right].updated = false;

      // mark this cell updated
      grid[idx_bot_right].updated = true;
      grid[idx_bot_right].updated = true;
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

  const uint idx = (row + 0) * width + (col + 0);
  const uint type = (int)grid[idx].type; // scale up from std::uint8_t

  // write texture
  // attributes go here
  const float4 out_color = {type, 1.f, 0.f, 1.f};
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

  const uint idx = (row + 0) * width + (screen_col + 0);
  const float d =
      sqrt(pow(col - (mouse_norm[0]), 2) + pow(row - mouse_norm[1], 2));

  if (d <= paint_radius && (VACANT(grid[idx]) || (target == AIR_TYPE))) {
    SET_CELL(next_grid[idx], target);
    SET_CELL(grid[idx], target);

    next_grid[idx].updated = false;
    grid[idx].updated = false;
  }
}
