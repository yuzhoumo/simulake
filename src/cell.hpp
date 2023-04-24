#pragma once

#include <memory>
#include <utility>

class Grid;
enum class CellType { AIR, WATER, OIL, SAND, FIRE, JELLO, SMOKE };

/* represents an individual CA-cell */
class BaseCell {
public:
  typedef std::tuple<int, int, int, int, int, int, int, int> Context;
  typedef std::tuple<int, int> Position;

  /* step forward simulation by 1 step, takes */
  // static void step(const Context &context, const Grid &grid);

private:
  static Context get_cell_context(const Position &);

  // make this an abstract class
  BaseCell() = delete;
};

/* individual datatypes / behaviors */

class AirCell : public BaseCell {
  static void step(const Position &, const Grid &);
};

class WaterCell : public BaseCell {
  static void step(const Position &, const Grid &);
};

class OilCell : public BaseCell {
  static void step(const Position &, const Grid &);
};

class SandCell : public BaseCell {
  static void step(const Position &, const Grid &);
};

class FireCell : public BaseCell {
  static void step(const Position &, const Grid &);
};

class JelloCell : public BaseCell {
  static void step(const Position &, const Grid &);
};

class SmokeCell : public BaseCell {
  static void step(const Position &, const Grid &);
};
