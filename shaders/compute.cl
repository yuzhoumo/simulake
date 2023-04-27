// vim: ft=cpp :

#define NONE_TYPE 0
#define AIR_TYPE 1
#define WATER_TYPE 4
#define SAND_TYPE 6

#define FALLS_DOWN(x) (x > WATER_TYPE)
#define VACANT(x) (x == AIR_TYPE)

__kernel void initialize(__global char *grid, __global char *next_grid,
                         const unsigned int width, const unsigned int height) {
  const unsigned int id = get_global_id(0);
  const unsigned int num_cells = width * height;
  if (id >= num_cells)
    return;

  const unsigned int size = get_global_size(0); // chunk size
  const unsigned int base_row = get_global_id(0);
  const unsigned int base_col = get_global_id(1);

  for (unsigned int row = base_row; row < min(base_row + size, height);
       row += 1) {
    for (unsigned int col = base_col; col < min(base_col + size, width);
         col += 1) {

      const unsigned int idx = row * width + col;
      grid[idx] = AIR_TYPE;
      next_grid[idx] = AIR_TYPE;
    }
  }
}

__kernel void simulate(__global char *grid, __global char *next_grid,
                       const unsigned int width, const unsigned int height) {
  const unsigned int id = get_global_id(0);
  const unsigned int num_cells = width * height;
  if (id >= num_cells)
    return;

  const unsigned int type = (int)grid[0];       // scale up from std::uint8_t
  const unsigned int size = get_global_size(0); // chunk size
  const unsigned int base_row = get_global_id(0);
  const unsigned int base_col = get_global_id(1);

  for (unsigned int row = base_row; row < min(base_row + size, height);
       row += 1) {
    for (unsigned int col = base_col; col < min(base_col + size, width);
         col += 1) {

      const unsigned int idx = row * width + col;
      const unsigned int idx_bot_left = (row + 1) * width + (col - 1);
      const unsigned int idx_bot = (row + 1) * width + (col);
      const unsigned int idx_bot_right = (row + 1) * width + (col + 1);
      const unsigned int idx_left = (row)*width + (col - 1);
      const unsigned int idx_right = (row)*width + (col + 1);
      const unsigned int idx_top_left = (row - 1) * width + (col - 1);
      const unsigned int idx_top = (row - 1) * width + (col);
      const unsigned int idx_top_right = (row - 1) * width + (col + 1);

      if (type == SAND_TYPE) {
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
  }
}
