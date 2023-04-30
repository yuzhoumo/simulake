#include "app.hpp"

namespace simulake {

App::App(std::uint32_t width, std::uint32_t height, std::uint32_t cell_size,
         const std::string_view title)
    : window(width, height, title), renderer(width, height, cell_size),
      device_grid(width / cell_size, height / cell_size, cell_size),
      grid(width / cell_size, height / cell_size) { // TODO(joe): merge grids
  state = &AppState::get_instance();
  state->set_time(glfwGetTime());

  /* note(joe): actual window size may differ from `width` & `height` if it
   * doesn't fit the screen, so add additional query on app instantiation. */
  const auto& size = window.get_window_size();
  state->set_window_size(std::get<0>(size), std::get<1>(size));
}

void App::update_device_grid() noexcept {
  const auto target_type =
      state->erase_mode ? CellType::AIR : state->selected_cell_type;

  if (state->mouse_pressed and target_type != CellType::NONE) {
    device_grid.spawn_cells({state->prev_mouse_x, state->prev_mouse_y},
                            static_cast<float>(state->spawn_radius),
                            target_type);
  }
}

void App::update_grid() noexcept {
  if (state->mouse_pressed and state->selected_cell_type != CellType::NONE) {
    std::uint32_t x = static_cast<std::uint32_t>(
      grid.get_width() * (state->prev_mouse_x / state->window_width));
    std::uint32_t y = static_cast<std::uint32_t>(
      grid.get_height() * (state->prev_mouse_y / state->window_height));

    grid.spawn_cells(x, y, state->spawn_radius, state->selected_cell_type);
  }
}

void App::run_gpu_sim() noexcept {
#if DEBUG
  // stats
  std::uint64_t frame_count = 0;
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
#endif

  device_grid.initialize_random(); // FIXME: test
  renderer.submit_grid(device_grid);

  while (!window.should_close()) {

#if DEBUG
    frame_count += 1;
#endif

    /* push frame */
    renderer.render();
    window.swap_buffers();

    /* update state */
    state->set_time(glfwGetTime());

    /* handle inputs */
    glfwPollEvents();
    update_device_grid();

    /* step simulation */
    device_grid.simulate();
  }

#if DEBUG
  const auto delta = (std::chrono::high_resolution_clock::now() - start);
  const auto duration_s =
      std::chrono::duration_cast<std::chrono::seconds>(delta);
  std::cout << "average fps: " << frame_count / duration_s.count() << std::endl;
#endif
}

void App::run_cpu_sim() noexcept {
#if DEBUG
  std::uint64_t frame_count = 0;
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
#endif

  renderer.submit_grid(grid);

  while (!window.should_close()) {

#if DEBUG
    frame_count += 1;
#endif

    /* push frame */
    renderer.render();
    window.swap_buffers();

    /* update state */
    state->set_time(glfwGetTime());

    /* handle inputs */
    glfwPollEvents();
    update_grid();

    /* step simulation */
    grid.simulate();
    renderer.submit_grid(grid);
  }

#if DEBUG
  const auto delta = (std::chrono::high_resolution_clock::now() - start);
  const auto duration_s =
      std::chrono::duration_cast<std::chrono::seconds>(delta);
  std::cout << "average fps: " << frame_count / duration_s.count() << std::endl;
#endif
}

void App::run(const bool gpu_mode) noexcept {
  int rc = glfwInit();
  assert(rc != 0);

  if (gpu_mode)
    run_gpu_sim();
  else
    run_cpu_sim();
}

} /* namespace simulake */
