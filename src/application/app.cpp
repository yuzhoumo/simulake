#include <functional>
#include "app.hpp"

namespace simulake {

App::App(std::uint32_t width, std::uint32_t height, std::uint32_t cell_size,
         const std::string_view title)
    : window(width, height, title), renderer(width, height, cell_size),
      device_grid(width / cell_size, height / cell_size, cell_size),
      grid(width / cell_size, height / cell_size) { // TODO(joe): merge grids

  state = &AppState::get_instance();
  state->renderer = &renderer;
  state->window = &window;
  state->grid = &grid;
  state->device_grid = &device_grid;
  state->set_time(window.get_time());
  state->set_cell_size(cell_size);

  /* set spawn radius approx. 10% of grid size */
  state->set_spawn_radius(std::min(width / cell_size, height / cell_size) / 10);

  /* note(joe): actual window size may differ from `width` & `height` if it
   * doesn't fit the screen, so add additional query on app instantiation. */
  const auto& size = window.get_window_size();
  state->set_window_size(std::get<0>(size), std::get<1>(size));
}

void App::step_gpu_sim() noexcept {
  const auto target_type = state->get_target_type();

  if (state->mouse_pressed and target_type != CellType::NONE) {
    device_grid.spawn_cells({state->prev_mouse_x, state->prev_mouse_y},
                            static_cast<float>(state->spawn_radius),
                            target_type);
  }

  device_grid.simulate();
}

void App::step_cpu_sim() noexcept {
  const auto target_type = state->get_target_type();

  if (state->mouse_pressed and target_type != CellType::NONE) {
    std::uint32_t x = static_cast<std::uint32_t>(
      grid.get_width() * (state->prev_mouse_x / state->window_width));
    std::uint32_t y = static_cast<std::uint32_t>(
      grid.get_height() * (state->prev_mouse_y / state->window_height));

    grid.spawn_cells(x, y, state->spawn_radius, target_type);
  }

  grid.simulate();
  renderer.submit_grid(grid);
}

void App::run(const bool gpu_mode) noexcept {

  /* init grid and simulation update function */
  std::function<void()> step_sim_func;
  if (gpu_mode) {
    renderer.submit_grid(device_grid);
    step_sim_func = std::bind(&App::step_gpu_sim, this);
  } else {
    renderer.submit_grid(grid);
    step_sim_func = std::bind(&App::step_cpu_sim, this);
  }

#if DEBUG
  std::uint64_t frame_count = 0;
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
#endif

  /* main render loop */
  while (!window.should_close()) {

#if DEBUG
    frame_count += 1;
#endif

    /* update app state */
    state->set_time(window.get_time());
    window.poll_events();

    /* step the simulation */
    if (!state->get_paused()) step_sim_func();

    /* push frame */
    renderer.render();
    window.swap_buffers();
  }

#if DEBUG
  const auto delta = (std::chrono::high_resolution_clock::now() - start);
  const auto duration_s =
      std::chrono::duration_cast<std::chrono::seconds>(delta);
  std::cout << "average fps: " << frame_count / duration_s.count() << std::endl;
#endif
}

} /* namespace simulake */
