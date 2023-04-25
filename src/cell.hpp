#pragma once

#include <algorithm>
#include <memory>
#include <utility>

namespace simulake {
class Grid;

#define FALLS_DOWN(x) (static_cast<std::uint8_t>(x) > 3)
#define VACANT(x) (static_cast<std::uint8_t>(x) == 1)

/* types a grid cell can have */
enum class CellType : std::uint8_t {
  // out of bounds
  NONE = 0,
  AIR,
  SMOKE,
  FIRE,
  WATER,
  OIL,
  SAND,
  JELLO,
};

/* represents an individual grid cell */
class BaseCell {
public:
  typedef std::tuple<std::uint32_t, std::uint32_t> position_t;

  // NOTE(vir): using struct instead of tuple for named access
  struct context_t {
    // clang-format off
    CellType top_left     = CellType::NONE;
    CellType top          = CellType::NONE;
    CellType top_right    = CellType::NONE;
    CellType left         = CellType::NONE;
    CellType right        = CellType::NONE;
    CellType bottom_left  = CellType::NONE;
    CellType bottom       = CellType::NONE;
    CellType bottom_right = CellType::NONE;
    // clang-format on
  };

  /* step forward simulation by 1 step, return new state of cell */
  // static CellType step(const Context &context, const Grid &grid) noexcept;

  [[nodiscard]] static context_t get_cell_context(const position_t &,
                                                  const Grid &) noexcept;

private:
  // make this an abstract class
  BaseCell() = delete;
};

/* individual datatypes / behaviors */

struct AirCell : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct WaterCell : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct OilCell : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct SandCell : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct FireCell : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct JelloCell : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

struct SmokeCell : public BaseCell {
  static void step(const position_t &, Grid &) noexcept;
};

} // namespace simulake
