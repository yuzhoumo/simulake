#include <algorithm>

#include "appstate.hpp"
#include "callbacks.hpp"

#include "../simulake/cell.hpp"
#include "../utils.hpp"

namespace simulake {
namespace callbacks {

void error(int errorcode, const char *description) {
  std::cerr << description << std::endl;
}

void key(GLFWwindow *window, int key, int scancode, int action, int mods) {
  AppState &state = AppState::get_instance();

  /* close window with escape key */
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  /* select a cell type */
  if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    state.selected_cell_type = simulake::CellType::AIR;
  if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    state.selected_cell_type = simulake::CellType::SMOKE;
  if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    state.selected_cell_type = simulake::CellType::FIRE;
  if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    state.selected_cell_type = simulake::CellType::WATER;
  if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    state.selected_cell_type = simulake::CellType::OIL;
  if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    state.selected_cell_type = simulake::CellType::SAND;
  if (key == GLFW_KEY_6 && action == GLFW_PRESS)
    state.selected_cell_type = simulake::CellType::JELLO;

  /* debug: print app state to console */
  if (key == GLFW_KEY_P && action == GLFW_PRESS)
    std::cout << state;
}

void cursor_pos(GLFWwindow *window, double xpos, double ypos) {
  AppState &state = AppState::get_instance();
  state.set_mouse_pos(xpos, ypos);
}

void mouse_button(GLFWwindow *window, int button, int action, int mods) {
  AppState &state = AppState::get_instance();

  if (button == GLFW_MOUSE_BUTTON_LEFT and !(mods & GLFW_MOD_SHIFT)) {
    if (action == GLFW_PRESS) {
      state.set_mouse_pressed(true);
    } else if (action == GLFW_RELEASE) {
      state.set_mouse_pressed(false);
    }
  }

  else if (button == GLFW_MOUSE_BUTTON_LEFT and (mods & GLFW_MOD_SHIFT)) {
    if (action == GLFW_PRESS) {
      state.set_mouse_pressed(true, true);
    } else if (action == GLFW_RELEASE) {
      state.set_mouse_pressed(false, false);
    }
  }
}

void scroll(GLFWwindow *window, double xoffset, double yoffset) {
  AppState &state = AppState::get_instance();
  int offset = state.spawn_radius + static_cast<int>(yoffset);
  int min_dim = std::min(state.window_width, state.window_height) / 4;
  offset = std::clamp(offset, 1, min_dim + 1);
  state.set_spawn_radius(static_cast<std::uint32_t>(offset));
}

void framebuffer_size(GLFWwindow *window, int width, int height) {
  AppState &state = AppState::get_instance();
  state.set_window_size(width, height);
}

} /* namespace callbacks */
} /* namespace simulake */
