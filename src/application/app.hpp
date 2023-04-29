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
  void run() noexcept;

private:
  AppState *state;
  Window window;

  DeviceGrid grid;
  Renderer renderer;
};

} /* namespace simulake */
#endif
