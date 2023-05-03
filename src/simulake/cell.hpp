#ifndef SIMULAKE_CELL_HPP
#define SIMULAKE_CELL_HPP

#include <algorithm>
#include <memory>
#include <utility>

namespace simulake {
class Grid;

/* types a grid cell can have */
enum class CellType : std::uint8_t {
  NONE = 0, // out of bounds
  AIR,
  SMOKE,
  FIRE,
  WATER,
  OIL,
  SAND,
  JELLO,
  STONE,
};

#define FALLS_DOWN(x) (x > simulake::CellType::WATER))
#define IS_FLUID(x) (static_cast<std::uint8_t>(x) > 0 \
                        && static_cast<std::uint8_t>(x) <= 5)

#define IS_AIR(x) (static_cast<std::uint8_t>(x) == 1)

#define IS_FLAMMABLE(x) (static_cast<std::uint8_t>(x) > 0 \
                            && (static_cast<std::uint8_t>(x) == 5 \
                            || static_cast<std::uint8_t>(x) == 6))

/* represents an individual grid cell */
class BaseCell {
public:

  typedef std::tuple<std::uint32_t, std::uint32_t> position_t;

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

  // not pure virtual becase we want want to use static functions
  /* step forward simulation by 1 step, return new state of cell */
  // static CellType step(const position_t &pos, Grid &grid) noexcept = 0;

  [[nodiscard]] static inline context_t get_cell_context(const position_t &,
        const Grid &) noexcept;

  // needed for any base class
  virtual ~BaseCell() = default;

private:
  // make this an abstract class
  BaseCell() = delete;
};

/* individual datatypes / behaviors */

struct AirCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct WaterCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
  static constexpr float max_mass = 1.0f;
  static constexpr float max_compress = 0.02f;
  static constexpr float min_mass = 0.0001f;
  static constexpr float max_speed = 1.0f;
  static constexpr float min_flow = 0.01f;

  /* Returns the amount of water that should be in the bottom cell. */
  static float get_stable_state_b(float total_mass) {
    if (total_mass <= 1) {
      return 1;
    } else if (total_mass < 2 * max_mass + max_compress) {
      return (max_mass * max_mass + total_mass * max_compress)
          / (max_mass + max_compress);
    } else {
      return (total_mass + max_compress) / 2;
    }
  }
};

struct OilCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct SandCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
  static constexpr bool isFlammable = true;
};

struct FireCell final : public BaseCell {
  static constexpr float mass_decay = 0.05;
  static void helper(CellType curr, Grid &grid, int x, int y, float remaining_mass);
  static void step(const position_t &, Grid &) noexcept;
};

struct JelloCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct SmokeCell final : public BaseCell {
  static constexpr float mass_decay = 0.001;
  static std::vector<position_t> getEmptyTopNeighbors(const position_t &, Grid &) noexcept;
  static std::vector<BaseCell::position_t> getEmptyBottomNeighbors(const position_t &, Grid &) noexcept;
  static void step(const position_t &, Grid &) noexcept;
};

struct StoneCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

} /* namespace simulake */

#endif
