#include "appstate.hpp"

namespace simulake {

void AppState::set_cell_size(const std::uint32_t cell_size) noexcept {
  AppState &state = AppState::get_instance();
  state.cell_size = cell_size;
}

void AppState::set_window_size(const float width, const float height) noexcept {
  AppState &state = AppState::get_instance();
  state.window_width = width;
  state.window_height = height;
}

void AppState::set_mouse_pos(const float xpos, const float ypos) noexcept {
  AppState &state = AppState::get_instance();
  state.prev_mouse_x = xpos;
  state.prev_mouse_y = ypos;
}

void AppState::set_mouse_pressed(const bool pressed, const bool erase_mode) noexcept {
  AppState &state = AppState::get_instance();
  state.mouse_pressed = pressed;
  state.erase_mode = erase_mode;
}

void AppState::set_time(const float curr_time) noexcept {
  AppState &state = AppState::get_instance();
  state.delta_time = curr_time - state.prev_time;
  state.prev_time = state.time;
  state.time = curr_time;
}

void AppState::set_selected_cell_type(const simulake::CellType type) noexcept {
  AppState &state = AppState::get_instance();
  state.selected_cell_type = type;
}

void AppState::set_spawn_radius(const std::uint32_t num_cells) noexcept {
  AppState &state = AppState::get_instance();
  state.spawn_radius = num_cells;
}

} /* namespace simulake */
