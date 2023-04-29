#include "appstate.hpp"

namespace simulake {

void AppState::set_cell_size(uint32_t cell_size) {
  AppState& state = AppState::get_instance();
  state.cell_size = cell_size;
}

void AppState::set_window_size(float width, float height) {
  AppState& state = AppState::get_instance();
  state.window_width = width;
  state.window_height = height;
}

void AppState::set_mouse_pos(float xpos, float ypos) {
  AppState& state = AppState::get_instance();
  state.prev_mouse_x = xpos;
  state.prev_mouse_y = ypos;
}

void AppState::set_time(float curr_time) {
  AppState& state = AppState::get_instance();
  state.delta_time = curr_time - state.prev_time;
  state.prev_time = state.time;
  state.time = curr_time;
}

void AppState::set_selected_cell_type(simulake::CellType type) {
  AppState& state = AppState::get_instance();
  state.selected_cell_type = type;
}

void AppState::set_spawn_radius(uint32_t num_cells) {
  AppState& state = AppState::get_instance();
  state.spawn_radius = num_cells;
}

} /* namespace simulake */
