// vim: ft=cpp :

#define NONE_TYPE 0
#define AIR_TYPE 1
#define WATER_TYPE 4
#define SAND_TYPE 6

#define FALLS_DOWN(x) (x > WATER_TYPE)
#define VACANT(x) (x == AIR_TYPE)

__kernel void initialize(__global char *grid, __global char *next_grid,
                         uint2 dims) {
  // const unsigned int width, const unsigned int height;
  const uint width = dims[0];
  const uint height = dims[1];

  const uint size = get_global_size(0); // full grid size (rows)
  const uint col = get_global_id(0);
  const uint row = get_global_id(1);

  const unsigned int idx = row * width + col;

  // printf("%d-%d-%d\n", row, col, idx);

  grid[idx] = AIR_TYPE;
  next_grid[idx] = AIR_TYPE;
}

__kernel void random_init(__global char *grid, const __global char *next_grid,
                          const uint2 dims) {
  const uint col = get_global_id(0);
  const uint row = get_global_id(1);
  const uint width = dims[0];
  const uint height = dims[1];

  // get a random number using xorshift
  const uint seed = 1337 + row;
  const uint t = seed ^ (seed << 11);
  const uint rand = (7331 + col) ^ ((7331 + col) >> 19) ^ (t ^ (t >> 8));

  if (rand % 5 == 0) {
    const unsigned int idx = row * width + col;
    grid[idx] = SAND_TYPE;
  }
}

__kernel void simulate(__global const char *grid, __global char *next_grid,
                       const uint2 dims) {
  const uint col = get_global_id(0); // <= local grid size (cols)
  const uint row = get_global_id(1); // <= local grid size (rows)

  const uint width = dims[0];
  const uint height = dims[1];
  const uint num_cells = width * height;

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

  const uint type = (uint)grid[idx];              // scale up from std::uint8_t
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

__kernel void render_texture(__write_only image2d_t texture,
                             __global const char *grid,
                             __global const char *next_grid, const uint2 dims,
                             const uint cell_size) {
  const uint col = get_global_id(0); // <= local grid size (cols)
  const uint row = get_global_id(1); // <= local grid size (rows)

  const uint width = dims[0];
  const uint height = dims[1];

  const uint idx = (row + 0) * width + (col + 0);
  const uint type = (int)grid[idx]; // scale up from std::uint8_t

  // write texture
  // attributes go here
  const float4 out_color = {type, 1.f, 0.f, 1.f};
  const int2 out_coord = {width - col, height - row};
  write_imagef(texture, out_coord, out_color);
}

__kernel void spawn_cells(__global const char *grid, __global char *next_grid,
                          const float2 mouse, const float paint_radius,
                          const uint target, const uint2 dims) {
  const uint col = get_global_id(0); // <= local grid size (cols)
  const uint row = get_global_id(1); // <= local grid size (rows)
  const uint width = dims[0];
  const uint height = dims[1];

  const uint idx = (row + 0) * width + (col + 0);
  const float d =
      sqrt(fabs(pow(mouse[0] - col, 2)) + fabs(pow(mouse[1] - row, 2)));

  if (d <= paint_radius && VACANT(grid[idx])) {
    next_grid[idx] = target;
  }
}
