#ifndef SIMULAKE_CELL_HPP
#define SIMULAKE_CELL_HPP

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <random>
#include <utility>

namespace simulake {
class Grid;

enum class CellType : std::uint8_t {
  NONE = 0, /* out of bounds */
  AIR,
  SMOKE,
  FIRE,
  GREEK_FIRE,
  WATER,
  OIL,
  SAND,
  JET_FUEL,
  STONE,
};

struct cell_data_t {
  CellType type = CellType::NONE; /* type (NONE means out of bounds) */
  float mass = 0.0f;              /* current mass of the cell */
  glm::vec2 velocity{0.0f};       /* current velocity */
  bool updated = false;           /* updated this frame */
};

class BaseCell {
public:
  virtual ~BaseCell() = default;

  /* cell constants */
  static constexpr float gravity = 10.f;

  /* get cell type properties */
  static inline bool is_liquid(CellType);
  static inline bool is_gas(CellType);
  static inline bool is_fluid(CellType);
  static inline float flammability(CellType);

  /* convenient packed representation of neighbors */
  struct __attribute__((packed)) context_t {
    // clang-format off
    CellType top_left     = CellType::NONE,
             top          = CellType::NONE,
             top_right    = CellType::NONE,
             left         = CellType::NONE,
             right        = CellType::NONE,
             bottom_left  = CellType::NONE,
             bottom       = CellType::NONE,
             bottom_right = CellType::NONE;
    // clang-format on
  };

  typedef std::tuple<std::uint32_t, std::uint32_t> position_t;

  static inline context_t get_cell_context(const position_t &,
                                           const Grid &) noexcept;

  static inline float random_float(float lower, float upper);
  static inline int random_int(int lower, int upper);

private:
  BaseCell() = delete;

  static std::random_device rd;
  static std::mt19937 gen;
};

/* air cell rules */
struct AirCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;
};

/* smoke cell rules */
struct SmokeCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;

  static constexpr float mass_decay = 0.005;
  static std::vector<position_t> getEmptyTopNeighbors(const position_t &,
                                                      Grid &) noexcept;
  static std::vector<BaseCell::position_t>
  getEmptyBottomNeighbors(const position_t &, Grid &) noexcept;
};

/* fire cell rules */
struct FireCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;

  static constexpr float mass_decay = 0.05f;
  static void helper(CellType curr, Grid &grid, int x, int y,
                     float remaining_mass);
};

/* water cell rules */
struct WaterCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;

  static constexpr float max_mass = 1.0f;
  static constexpr float max_compress = 0.02f;
  static constexpr float min_mass = 0.001f;
  static constexpr float max_speed = 1.0f;
  static constexpr float min_flow = 0.01f;

  /* Returns the amount of water that should be in the bottom cell. */
  static float get_stable_state_b(float total_mass) {
    if (total_mass <= 1) {
      return 1;
    } else if (total_mass < 2 * max_mass + max_compress) {
      return (max_mass * max_mass + total_mass * max_compress) /
             (max_mass + max_compress);
    } else {
      return (total_mass + max_compress) / 2;
    }
  }
};

/* oil cell rules */
struct OilCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;
};

/* sand cell rules */
struct SandCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;

  static constexpr bool isFlammable = true;
};

/* jet fuel cell rules */
struct JetFuelCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;
};

/* stone cell rules */
struct StoneCell final : public BaseCell {
  static cell_data_t spawn(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;
};

} /* namespace simulake */

#endif
