#include <chrono>
#include <iostream>

#include "utils.hpp"

#include "application/appstate.hpp"
#include "simulake/cell.hpp"
#include "simulake/grid.hpp"

std::ostream &operator<<(std::ostream &stream,
                         const simulake::AppState &state) {
  stream << "AppState:\n";
  stream << "  target cell type: " << state.get_target_type() << "\n";
  stream << "  simulation paused: " << state.get_paused() << "\n";
  stream << "  spawn radius: " << state.spawn_radius << "\n";
  stream << "  cell size: " << state.cell_size << "\n";
  stream << "  window width: " << state.window_width << "\n";
  stream << "  window height: " << state.window_height << "\n";
  stream << "  prev mouse x: " << state.prev_mouse_x << "\n";
  stream << "  prev mouse y: " << state.prev_mouse_y << "\n";
  stream << "  mouse pressed: " << state.mouse_pressed << "\n";
  stream << "  time: " << state.time << "\n";
  stream << "  prev time: " << state.prev_time << "\n";
  stream << "  delta time: " << state.delta_time << "\n";
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const simulake::CellType type) {
  switch (type) {
  case simulake::CellType::AIR:
    stream << 'A';
    break;
  case simulake::CellType::WATER:
    stream << 'W';
    break;
  case simulake::CellType::OIL:
    stream << 'O';
    break;
  case simulake::CellType::SAND:
    stream << 'S';
    break;
  case simulake::CellType::FIRE:
    stream << 'F';
    break;
  case simulake::CellType::JELLO:
    stream << 'J';
    break;
  case simulake::CellType::SMOKE:
    stream << '*';
    break;

  default:
    stream << '-';
    break;
  }

  return stream;
}

std::ostream &operator<<(std::ostream &stream,
                         const simulake::BaseCell::context_t context) {
  // clang-format off
  stream << context.top_left << " " << context.top << " " << context.top_right << '\n';
  stream << context.left << " _ " << context.right << '\n';
  stream << context.bottom_left << " " << context.bottom << " " << context.bottom_right << '\n';
  // clang-format on
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const simulake::Grid &grid) {
  for (int i = 0; i < grid.get_height(); i += 1) {
    for (int j = 0; j < grid.get_width(); j += 1)
      stream << grid.type_at(i, j) << ' ';

    stream << '\n';
  }
  return stream;
}

float constrain(float val, float low, float high) {
    if (val > high) return high;
    if (val < low) return low;
    return val;
}

namespace simulake {
scope_timer_t::scope_timer_t(const char *_title)
    : start(std::chrono::high_resolution_clock::now()), title(_title) {}

scope_timer_t::~scope_timer_t() {
  // clang-format off
  const auto delta = std::chrono::high_resolution_clock::now() - start;
  const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta);
  const auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(delta);
  const auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(delta);
  // clang-format on

  /* show milliseconds if > 0 */
  if (duration_ms.count() > 0)
    std::cout << title << " took: " << duration_ms.count() << "ms" << std::endl;

  /* else show microseconds if > 0 */
  else if (duration_us.count() > 0)
    std::cout << title << " took: " << duration_us.count() << "us" << std::endl;

  /* else show nano seconds */
  else
    std::cout << title << " took: " << duration_ns.count() << "ns" << std::endl;
}

} /* namespace simulake */
