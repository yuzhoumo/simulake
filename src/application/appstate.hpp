#ifndef APP_STATE_HPP
#define APP_STATE_HPP

#include "../simulake/cell.hpp"
#include "../simulake/renderer.hpp"
#include "../simulake/grid_base.hpp"
#include "window.hpp"

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

  /* set renderer pointer */
  static void set_renderer(Renderer *) noexcept;

  /* set window pointer */
  static void set_window(Window *) noexcept;

  /* set grid pointer */
  static void set_grid(GridBase *) noexcept;

  /* set/get the currently selected cell type */
  static void set_selected_cell_type(const simulake::CellType) noexcept;

  /* update the mouse interaction spawn radius (in cells) */
  static void set_spawn_radius(const std::uint32_t) noexcept;

  /* update the cell size (pixels) */
  static void set_cell_size(const std::uint32_t) noexcept;

  /* update bool tracking if mouse button is pressed down */
  static void set_mouse_pressed(const bool) noexcept;

  /* update boolean denoting erase mode */
  static void set_erase_mode(const bool) noexcept;

  /* update the window width and height */
  static void set_window_size(const std::uint32_t, const std::uint32_t) noexcept;

  /* update previous mouse position */
  static void set_mouse_pos(const float, const float) noexcept;

  /* update time values based on current time */
  static void set_time(const float) noexcept;

  /* set/get if simulation is paused */
  static void set_paused(const bool) noexcept;

  /* get current target cell type accounting for modifiers (e.g. erase mode) */
  static CellType get_target_type() noexcept;

  /* raw accessor methods for private variables */
  static Renderer *get_renderer() noexcept;
  static Window *get_window() noexcept;
  static GridBase *get_grid() noexcept;
  static simulake::CellType get_selected_cell_type() noexcept;
  static std::uint32_t get_spawn_radius() noexcept;
  static std::uint32_t get_cell_size() noexcept;
  static std::uint32_t get_window_width() noexcept;
  static std::uint32_t get_window_height() noexcept;
  static float get_prev_mouse_x() noexcept;
  static float get_prev_mouse_y() noexcept;
  static float get_time() noexcept;
  static float get_prev_time() noexcept;
  static float get_delta_time() noexcept;
  static bool is_mouse_pressed() noexcept;
  static bool is_erase_mode() noexcept;
  static bool is_paused() noexcept;

private:
  AppState() = default;

  GridBase *grid;
  Renderer *renderer;
  Window *window;

  simulake::CellType selected_cell_type = simulake::CellType::NONE;
  std::uint32_t spawn_radius = 20;
  std::uint32_t cell_size = 1;

  /* track height and width of the window */
  std::uint32_t window_width = 0;
  std::uint32_t window_height = 0;

  /* previous mouse coordinates */
  float prev_mouse_x = 0;
  float prev_mouse_y = 0;

  float time = 0.0f;       /* time at the current frame */
  float prev_time = 0.0f;  /* time at the previous frame */
  float delta_time = 0.0f; /* time between previous and current frame*/

  bool mouse_pressed = false;
  bool erase_mode = false;
  bool paused = false; /* pause the simulation when true */
};

} /* namespace simulake */

#endif
