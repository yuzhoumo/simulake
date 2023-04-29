#ifndef APP_HPP
#define APP_HPP

#include "../simulake/renderer.hpp"
#include "appstate.hpp"
#include "window.hpp"

namespace simulake {

class App {
public:
  App(const std::uint32_t width, const std::uint32_t height,
      const std::uint32_t cell_size, const std::string_view title);
  ~App() = default;

  /* run main render loop */
  void run(const bool) noexcept;

  /* update grid based on current app state */
  void update_grid() noexcept; // TODO(joe): merge these into one function after
                               //            grid class refactor
  void update_device_grid() noexcept;

  void run_gpu_sim() noexcept;
  void run_cpu_sim() noexcept;

private:
  AppState *state;
  Window window;

  Grid grid; // TODO(joe): inherit device_grid from grid into one class
             //            so that there is one grid variable
  DeviceGrid device_grid;
  Renderer renderer;
};

} /* namespace simulake */
#endif
