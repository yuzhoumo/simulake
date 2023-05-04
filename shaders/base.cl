// vim: ft=cpp :

#ifndef SIMULAKE_COMPUTE_BASE_CL
#define SIMULAKE_COMPUTE_BASE_CL

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

#define V_STATIONARY ((float2){0.0f, 0.0f})

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
#define   SMOKE_MASS   1.0f
#define   FIRE_MASS    1.0f
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
inline uint get_rand(const uint seed_, const uint row, const uint col,
                     const float mass) {
  // xorshift
  // const uint seed = seed + row;
  // const uint t = seed ^ (seed << 11);
  // return (col) ^ ((col) >> 19) ^ (t ^ (t >> 8));

  // java random
  ulong seed = seed_ + (row * col) * mass * get_local_id(0) - get_local_id(1);
  seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
  return seed >> 16;
}

inline float get_rand_float(const uint seed_, const uint row, const uint col,
                            const float mass) {
  return ((float)get_rand(seed_, row, col, mass)) / ((float)UINT_MAX);
}

#define SCALE_FLOAT(f, low, high) ((f - 1.0f) / (1.0f)) * (high - low) + low

inline float get_mass(const uint type, const uint random_seed, const uint row,
                      const uint col) {
  switch (type) {
  case SMOKE_TYPE:
    return get_rand_float(random_seed, row, col, SMOKE_MASS);
    break;
  case FIRE_TYPE:
    return SCALE_FLOAT(get_rand_float(random_seed, row, col, FIRE_MASS), 0.7f,
                       FIRE_MASS);
    break;
  case WATER_TYPE:
    return get_rand_float(random_seed, row, col, WATER_MASS);
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
  float2 velocity;

  bool updated;
} grid_t;

#define GEN_NEIGHBOUR_INDICES(row, col, width, height)                         \
  const uint idx_top_left = GET_INDEX(row - 1, col - 1, width, height);        \
  const uint idx_top = GET_INDEX(row - 1, col + 0, width, height);             \
  const uint idx_top_right = GET_INDEX(row - 1, col + 1, width, height);       \
  const uint idx_left = GET_INDEX(row + 0, col - 1, width, height);            \
  const uint idx = GET_INDEX(row + 0, col + 0, width, height);                 \
  const uint idx_right = GET_INDEX(row + 0, col + 1, width, height);           \
  const uint idx_bot_left = GET_INDEX(row + 1, col - 1, width, height);        \
  const uint idx_bot = GET_INDEX(row + 1, col + 0, width, height);             \
  const uint idx_bot_right = GET_INDEX(row + 1, col + 1, width, height);

#define GEN_BOUNDS_VALID(row, col, width, height)                              \
  const bool top_valid = ((long)row - 1) >= 0;                                 \
  const bool bot_valid = ((long)row + 1) < (long)height;                       \
  const bool left_valid = ((long)col - 1) > 0;                                 \
  const bool right_valid = ((long)col + 1) < (long)width;

#define GEN_LOC_VARS()                                                         \
  const uint col = get_global_id(0);                                           \
  const uint row = get_global_id(1);

#define STEP_IMPL(name)                                                        \
  inline void name(const uint2 loc, const uint2 dims, __global grid_t *grid,   \
                   __global grid_t *next_grid, uint rand_seed)

#define INVOKE_IMPL(name) name(loc, dims, grid, next_grid, rand_seed)

#define GEN_STEP_LOC()                                                         \
  const uint row = loc[0];                                                     \
  const uint col = loc[1];                                                     \
  const uint width = dims[0];                                                  \
  const uint height = dims[1];

#define GEN_STEP_IMPL_HEADER()                                                 \
  GEN_STEP_LOC();                                                              \
  GEN_BOUNDS_VALID(row, col, width, height);                                   \
  GEN_NEIGHBOUR_INDICES(row, col, width, height);                              \
  const uint type = (int)grid[idx].type;

#endif
