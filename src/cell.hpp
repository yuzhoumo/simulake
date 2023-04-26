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
};

#define FALLS_DOWN(x) (x > simulake::CellType::WATER))
#define VACANT(x) (x == simulake::CellType::AIR)

/* represents an individual grid cell */
class BaseCell {
public:
  typedef std::tuple<std::uint32_t, std::uint32_t> position_t;

  // convenient packed representation of neighbours
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

  [[nodiscard]] static context_t get_cell_context(const position_t &,
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
};

struct OilCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct SandCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct FireCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct JelloCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct SmokeCell final : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

} // namespace simulake

#endif
