#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include "app.hpp"

namespace simulake {

App::App(std::uint32_t width, std::uint32_t height, std::uint32_t cell_size,
         const std::string_view title)
    : window(width, height, title), renderer(width, height, cell_size),
      device_grid(width / cell_size, height / cell_size, cell_size),
      grid(width / cell_size, height / cell_size) { // TODO(joe): merge grids

  state = &AppState::get_instance();
  state->set_renderer(&renderer);
  state->set_window(&window);
  state->set_time(window.get_time());
  state->set_cell_size(cell_size);

  /* set spawn radius approx. 10% of grid size */
  state->set_spawn_radius(std::min(width / cell_size, height / cell_size) / 10);

  /* note(joe): actual window size may differ from `width` & `height` if it
   * doesn't fit the screen, so add additional query on app instantiation. */
  const auto &size = window.get_window_size();
  state->set_window_size(std::get<0>(size), std::get<1>(size));
}

void App::step_sim(bool paused, GridBase *sim_grid) noexcept {
  const auto target_type = state->get_target_type();

  if (state->is_mouse_pressed() and target_type != CellType::NONE) {
    std::uint32_t x = static_cast<std::uint32_t>(grid.get_width() *
        (state->get_prev_mouse_x() / state->get_window_width()));
    std::uint32_t y = static_cast<std::uint32_t>(grid.get_height() *
        (state->get_prev_mouse_y() / state->get_window_height()));

    sim_grid->spawn_cells({x, y}, state->get_spawn_radius(), target_type);
  }

  if (!paused) sim_grid->simulate();
}

void App::run(const bool gpu_mode) noexcept {

  /* init grid and simulation update function */
  GridBase *sim_grid = gpu_mode ? static_cast<GridBase *>(&device_grid) :
                                  static_cast<GridBase *>(&grid);

  renderer.submit_grid(sim_grid);

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
    state->set_time(window.get_time());
    window.poll_events();

    /* step the simulation */
    step_sim(state->is_paused(), sim_grid);
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

GridBase::serialized_grid_t load_grid(const std::string_view path) {
  std::filesystem::path file_path(path);

  if (!std::filesystem::exists(file_path)) {
    throw std::runtime_error("File does not exist.");
  }

  std::ifstream input_file(file_path, std::ios::binary);

  if (!input_file) {
    throw std::runtime_error("Failed to open file for reading.");
  }

  GridBase::serialized_grid_t grid;

  input_file.read(reinterpret_cast<char*>(&grid.width), sizeof(grid.width));
  input_file.read(reinterpret_cast<char*>(&grid.height), sizeof(grid.height));
  input_file.read(reinterpret_cast<char*>(&grid.stride), sizeof(grid.stride));

  std::size_t buffer_size = grid.stride * grid.height;
  grid.buffer.resize(buffer_size);
  input_file.read(reinterpret_cast<char*>(grid.buffer.data()), buffer_size * sizeof(float));

  if (!input_file) {
    throw std::runtime_error("Failed to read data from file.");
  }

  input_file.close();

  return grid;
}

void store_grid(const GridBase::serialized_grid_t& data,
                const std::string_view path) {

  std::filesystem::path file_path(path);

  if (file_path.empty()) {
    /* get time */
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                now.time_since_epoch()) % 1000;
    /* format time */
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_time_t), "%Y%m%d%H%M%S");
    oss << '_' << std::setfill('0') << std::setw(3) << now_ms.count();

    /* create base file path */
    std::string file_path_base = std::to_string(data.width) +
                  "x" + std::to_string(data.height) +
                  "x" + std::to_string(data.stride) +
                  "_" + oss.str();

    /* append count if file already exists */
    int counter = 0;
    do {
      file_path = std::filesystem::path(file_path_base +
                  (counter > 0 ? "_" + std::to_string(counter) : "") + ".dat");
      counter++;
    } while (std::filesystem::exists(file_path));
  }

  /* write binary file */
  std::ofstream output_file(file_path, std::ios::binary);

  if (!output_file) {
    throw std::runtime_error("Failed to open file for writing.");
  }

  output_file.write(reinterpret_cast<const char*>(&data.width), sizeof(data.width));
  output_file.write(reinterpret_cast<const char*>(&data.height), sizeof(data.height));
  output_file.write(reinterpret_cast<const char*>(&data.stride), sizeof(data.stride));

  std::size_t buffer_size = data.stride * data.height;
  output_file.write(reinterpret_cast<const char*>(data.buffer.data()),
                                                  buffer_size * sizeof(float));

  if (!output_file) {
    throw std::runtime_error("Failed to write data to file.");
  }

  output_file.close();
}

} /* namespace simulake */
