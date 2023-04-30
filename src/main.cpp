#include "application/graphics.hpp"
#include "application/app.hpp"
#include "test.hpp"

int main(int argc, char **argv) {
  constexpr auto WIDTH = 1920;
  constexpr auto HEIGHT = 1080;
  constexpr auto CELL_SIZE = 4;
  constexpr auto GPU_MODE = false;

  // simulake::test::test_renderer();
  // simulake::test::test_simulation();
  // simulake::test::test_device_grid();

  simulake::init_window_context();
  simulake::App app = simulake::App{WIDTH, HEIGHT, CELL_SIZE, "simulake"};

  {
    PROFILE_SCOPE("total run time");
    app.run(GPU_MODE);
  }

  return 0;
}
