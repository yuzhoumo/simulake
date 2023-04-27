// vim: ft=cpp :

#define NONE_TYPE 0
#define AIR_TYPE 1
#define WATER_TYPE 4
#define SAND_TYPE 6

#define FALLS_DOWN(x) (x > WATER_TYPE)
#define VACANT(x) (x == AIR_TYPE)

__kernel void initialize(__global char *grid, __global char *next_grid,
                         const unsigned int width, const unsigned int height) {
  const unsigned int size = get_global_size(0); // full grid size (rows)
  const unsigned int col = get_global_id(0);
  const unsigned int row = get_global_id(1);

  const unsigned int idx = row * width + col;

  // printf("%d-%d-%d\n", row, col, idx);

  grid[idx] = AIR_TYPE;
  next_grid[idx] = AIR_TYPE;
}

__kernel void random_init(__global char *grid, __global char *next_grid,
                          const unsigned int width, const unsigned int height) {
  const unsigned int col = get_global_id(0);
  const unsigned int row = get_global_id(1);

  const uint seed = 1337 + row;
  const uint t = seed ^ (seed << 11);
  const uint rand = (7331 + col) ^ ((7331 + col) >> 19) ^ (t ^ (t >> 8));

  if (rand % 5 == 0) {
    const unsigned int idx = row * width + col;
    grid[idx] = SAND_TYPE;
  }
}

__kernel void simulate(__global char *grid, __global char *next_grid,
                       const unsigned int width, const unsigned int height) {
  const unsigned int num_cells = width * height;
  const unsigned int col = get_global_id(0); // <= local grid size (cols)
  const unsigned int row = get_global_id(1); // <= local grid size (rows)

  // clang-format off
  const unsigned int idx_top_left  = (row - 1) * width + (col - 1);
  const unsigned int idx_top       = (row - 1) * width + (col + 0);
  const unsigned int idx_top_right = (row - 1) * width + (col + 1);
  const unsigned int idx_left      = (row + 0) * width + (col - 1);
  const unsigned int idx           = (row + 0) * width + (col + 0);
  const unsigned int idx_right     = (row + 0) * width + (col + 1);
  const unsigned int idx_bot_left  = (row + 1) * width + (col - 1);
  const unsigned int idx_bot       = (row + 1) * width + (col + 0);
  const unsigned int idx_bot_right = (row + 1) * width + (col + 1);
  // clang-format on

  const unsigned int type = (int)grid[idx];       // scale up from std::uint8_t
  if (type == SAND_TYPE && idx_bot < num_cells) { // cant fall below ground
    // printf("%d-%d-%d\n", grid > next_grid ? 0 : 1, idx, idx_bot);

    if (VACANT(grid[idx_bot])) {
      next_grid[idx_bot] = SAND_TYPE;
      next_grid[idx] = AIR_TYPE;
    }

    else if (VACANT(grid[idx_bot_left])) {
      next_grid[idx_bot_left] = SAND_TYPE;
      next_grid[idx] = AIR_TYPE;
    }

    else if (VACANT(grid[idx_bot_right])) {
      next_grid[idx_bot_right] = SAND_TYPE;
      next_grid[idx] = AIR_TYPE;
    }
  }
}

// TODO(vir): this doesnt work
__kernel void render_texture(__write_only image2d_t texture,
                             __global char *grid, __global char *next_grid,
                             const unsigned int width,
                             const unsigned int height,
                             const unsigned int cell_size) {
  const unsigned int num_cells = width * height;
  const unsigned int size = get_global_size(0); // == full grid size
  const unsigned int col = get_global_id(0);    // <= local grid size (cols)
  const unsigned int row = get_global_id(1);    // <= local grid size (rows)

  const unsigned int idx = (row + 0) * width + (col + 0);
  const unsigned int type = (int)grid[idx]; // scale up from
  const int2 out_coord = {row, col};

  // texture[idx] = type;
  float4 out_color = (float4)(0.0f, 0.0f, 0.0f, (float)type);
  write_imagef(texture, out_coord, (float)type);
}
