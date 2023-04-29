#ifndef APP_HPP
#define APP_HPP

#include "appstate.hpp"
#include "window.hpp"
#include "../simulake/renderer.hpp"

namespace simulake {

class App {
public:
  App(uint32_t width, uint32_t height, uint32_t cell_size,
      std::string_view title);

  ~App() {}

  /* run main render loop */
  void run();

private:
  AppState* state;
  Window window;

  DeviceGrid grid;
  Renderer renderer;
};

} /* namespace simulake */
#endif
