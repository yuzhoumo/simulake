#ifndef APP_STATE_HPP
#define APP_STATE_HPP

#include "../simulake/cell.hpp"

/* `AppState` is a singleton class used to store and update data required by
 * glfw callback functions. Because glfw takes static functions as callbacks,
 * we provide additional data through this singleton instead of polluting
 * global namespace with variables.
 */

namespace simulake {

class AppState {
public:
  [[nodiscard]] static AppState &get_instance() {
    static AppState instance{};
    return instance;
  }

  /* disable direct instantiation, copies, moves */
  AppState(const AppState &) = delete;
  void operator=(const AppState &) = delete;
  AppState(AppState &&) = delete;
  void operator=(AppState &&) = delete;

  /* update the cell size (pixels) */
  static void set_cell_size(const std::uint32_t) noexcept;

  /* update the window width and height */
  static void set_window_size(const float, const float) noexcept;

  /* update previous mouse position */
  static void set_mouse_pos(const float, const float) noexcept;

  /* update bool tracking if mouse button is pressed down */
  static void set_mouse_pressed(const bool, const bool = false) noexcept;

  /* update time values based on current time */
  static void set_time(const float) noexcept;

  /* update the currently selected cell type */
  static void set_selected_cell_type(const simulake::CellType) noexcept;

  /* update the mouse interaction spawn radius (in cells) */
  static void set_spawn_radius(const std::uint32_t) noexcept;

  /* simulake */
  simulake::CellType selected_cell_type = simulake::CellType::NONE;
  std::uint32_t spawn_radius = 50;
  std::uint32_t cell_size = 1;
  bool mouse_pressed = false;
  bool erase_mode = false;

  /* track height and width of the window */
  std::uint32_t window_width = 0;
  std::uint32_t window_height = 0;

  /* previous mouse coordinates */
  float prev_mouse_x = 0;
  float prev_mouse_y = 0;

  float time = 0.0f;       /* time at the current frame */
  float prev_time = 0.0f;  /* time at the previous frame */
  float delta_time = 0.0f; /* time between previous and current frame*/

private:
  AppState() = default;
};

} /* namespace simulake */

#endif