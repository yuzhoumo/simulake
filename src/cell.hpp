#pragma once

#include <memory>
#include <utility>

namespace simulake {
class Grid;

/* types a grid cell can have */
enum class CellType : std::uint8_t {
  AIR,
  WATER,
  OIL,
  SAND,
  FIRE,
  JELLO,
  SMOKE,
  NONE
};

/* represents an individual grid cell */
class BaseCell {
public:
  typedef std::tuple<std::uint32_t, std::uint32_t> position_t;
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

  /* step forward simulation by 1 step, takes */
  // static void step(const Context &context, const Grid &grid) noexcept;

private:
  [[nodiscard]] static context_t get_cell_context(const position_t &,
                                                  const Grid &) noexcept;

  // make this an abstract class
  BaseCell() = delete;
};

/* individual datatypes / behaviors */

struct AirCell : public BaseCell {
  static void step(const position_t &, const Grid &) noexcept;
};

struct WaterCell : public BaseCell {
  static void step(const position_t &, const Grid &) noexcept;
};

struct OilCell : public BaseCell {
  static void step(const position_t &, const Grid &) noexcept;
};

struct SandCell : public BaseCell {
  static void step(const position_t &, const Grid &) noexcept;
};

struct FireCell : public BaseCell {
  static void step(const position_t &, const Grid &) noexcept;
};

struct JelloCell : public BaseCell {
  static void step(const position_t &, const Grid &) noexcept;
};

struct SmokeCell : public BaseCell {
  static void step(const position_t &, const Grid &) noexcept;
};
} // namespace simulake
