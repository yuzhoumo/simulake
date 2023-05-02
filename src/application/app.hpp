#ifndef APP_HPP
#define APP_HPP

#include "../simulake/renderer.hpp"
#include "../simulake/grid_base.hpp"
#include "../simulake/device_grid.hpp"
#include "../simulake/grid.hpp"
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

private:
  void step_sim(bool, GridBase *) noexcept;

  AppState *state;
  Window window;
  Renderer renderer;

  Grid grid;
  DeviceGrid device_grid;
};

} /* namespace simulake */
#endif
