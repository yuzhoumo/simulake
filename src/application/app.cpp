#include "app.hpp"

namespace simulake {

App::App(uint32_t width, uint32_t height, uint32_t cell_size,
         std::string_view title) :
            window(width, height, title),
            renderer(width, height, cell_size),
            grid(width / cell_size, height / cell_size, cell_size) {

  state = &AppState::get_instance();
  state->set_window_size(width, height);
  state->set_time(glfwGetTime());
}

void App::run() {
  grid.initialize_random(); // FIXME: test
  renderer.submit_grid(grid);

  while (!window.should_close()) {
    renderer.render();
    window.swap_buffers();

    grid.simulate();
    renderer.submit_grid(grid);

    glfwPollEvents();
    state->set_time(glfwGetTime());
  }
}

} /* namespace simulake */
