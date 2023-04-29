#include "application/graphics.hpp"
#include "application/app.hpp"
#include "test.hpp"

int main(int argc, char **argv) {
  constexpr auto WIDTH = 1280;
  constexpr auto HEIGHT = 720;
  constexpr auto CELL_SIZE = 4;
  constexpr auto GPU_MODE = false;

  int rc = glfwInit();
  assert(rc != 0);

  // simulake::test::test_renderer();
  // simulake::test::test_simulation();
  // simulake::test::test_device_grid();

  simulake::App app = simulake::App{WIDTH, HEIGHT, CELL_SIZE, "simulake"};

  {
    PROFILE_SCOPE("total run time");
    app.run(GPU_MODE);
  }

  return 0;
}
