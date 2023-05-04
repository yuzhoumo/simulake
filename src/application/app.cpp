#include "app.hpp"
#include "loader.hpp"

namespace simulake {

App::App(std::uint32_t width, std::uint32_t height, std::uint32_t cell_size,
         const std::string_view title)
    : window(width * cell_size, height * cell_size, title),
      renderer(width, height, cell_size),
      device_grid(width, height, cell_size),
      grid(width, height),
      state(AppState::get_instance()) {

  state.set_renderer(&renderer);
  state.set_window(&window);
  state.set_time(window.get_time());
  state.set_cell_size(cell_size);

  /* set spawn radius approx. 10% of grid size */
  state.set_spawn_radius(std::min(width, height) / 10);

  /* note(joe): actual window size may differ from parameters if it
   * doesn't fit the screen, so add additional query on app instantiation. */
  const auto &size = window.get_window_size();
  state.set_window_size(std::get<0>(size), std::get<1>(size));
}

void App::step_sim(bool paused, GridBase *sim_grid) noexcept {
  const auto target_type = state.get_target_type();

  if (state.is_mouse_pressed() and target_type != CellType::NONE) {
    std::uint32_t x = static_cast<std::uint32_t>(grid.get_width() *
        (state.get_prev_mouse_x() / state.get_window_width()));
    std::uint32_t y = static_cast<std::uint32_t>(grid.get_height() *
        (state.get_prev_mouse_y() / state.get_window_height()));

    sim_grid->spawn_cells({x, y}, state.get_spawn_radius(), target_type);
  }

  if (!paused) sim_grid->simulate(state.get_delta_time());
}

void App::run(const bool gpu_mode, GridBase::serialized_grid_t *data) noexcept {

  /* init grid and simulation update function */
  GridBase *sim_grid = gpu_mode ? static_cast<GridBase *>(&device_grid) :
                                  static_cast<GridBase *>(&grid);

  state.set_grid(sim_grid);
  if (data != nullptr) {
    // TODO(vir): MAKE desearlize return true/false on error or hard exit is required
    state.get_grid()->deserialize(*data);
    state.set_paused(true);
  }

  renderer.submit_grid(sim_grid);

  if (gpu_mode) {
    renderer.submit_grid(sim_grid);
  }

  if (gpu_mode) {
    renderer.submit_grid(device_grid);
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
    state.set_time(window.get_time());
    window.poll_events();

    /* step the simulation */
    step_sim(state.is_paused(), sim_grid);
    if (!gpu_mode) renderer.submit_grid(sim_grid);

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
