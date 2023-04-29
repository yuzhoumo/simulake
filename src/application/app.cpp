#include "app.hpp"

namespace simulake {

App::App(std::uint32_t width, std::uint32_t height, std::uint32_t cell_size,
         const std::string_view title)
    : window(width, height, title), renderer(width, height, cell_size),
      grid(width / cell_size, height / cell_size, cell_size) {
  state = &AppState::get_instance();
  state->set_window_size(width, height);
  state->set_time(glfwGetTime());
}

void App::run() noexcept {
  grid.initialize_random(); // FIXME: test
  renderer.submit_grid(grid);

#if DEBUG
  // stats
  std::uint64_t frame_count = 0;
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
#endif

  while (!window.should_close()) {
#if DEBUG
    frame_count += 1;
#endif

    renderer.render();
    window.swap_buffers();

    grid.simulate();
    renderer.submit_grid(grid);

    glfwPollEvents();
    state->set_time(glfwGetTime());
  }

#if DEBUG
  const auto delta = (std::chrono::high_resolution_clock::now() - start);
  const auto duration_s =
      std::chrono::duration_cast<std::chrono::seconds>(delta);
  std::cout << "average fps: " << frame_count / duration_s.count() << std::endl;
#endif
}

} /* namespace simulake */
