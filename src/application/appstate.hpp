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
  static AppState& get_instance() {
    static AppState instance;
    return instance;
  }

  AppState(const AppState&) = delete;
  void operator=(const AppState&) = delete;

  /* update the cell size (pixels) */
  static void set_cell_size(uint32_t cell_size);

  /* update the window width and height */
  static void set_window_size(float width, float height);

  /* update previous mouse position */
  static void set_mouse_pos(float xpos, float ypos);

  /* update time values based on current time */
  static void set_time(float curr_time);

  /* update the currently selected cell type */
  static void set_selected_cell_type(simulake::CellType type);

  /* update the mouse interaction spawn radius (in cells) */
  static void set_spawn_radius(uint32_t num_cells);

  /* simulake */
  simulake::CellType selected_cell_type = simulake::CellType::NONE;
  uint32_t spawn_radius = 1;
  uint32_t cell_size = 1;

  /* track height and width of the window */
  uint32_t window_width;
  uint32_t window_height;

  /* previous mouse coordinates */
  float prev_mouse_x;
  float prev_mouse_y;

  float time = 0.0f;        /* time at the current frame */
  float prev_time = 0.0f;   /* time at the previous frame */
  float delta_time = 0.0f;  /* time between previous and current frame*/

private:
  AppState() {}
};

} /* namespace simulake */

#endif
