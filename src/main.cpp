#include <filesystem>
#include <iostream>
#include <thread>

#include <omp.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "constants.hpp"
#include "renderer.hpp"
#include "shader.hpp"

#include "grid.hpp"
#include "utils.hpp"

void test_renderer(int argc, char **argv) {
  PROFILE_FUNCTION();
  constexpr auto WIDTH = 800;
  constexpr auto HEIGHT = 600;
  constexpr auto CELL_SIZE = 2;

  // TODO(vir): find a better place for this initialization
  { assert(glfwInit()); }

  simulake::Renderer renderer(WIDTH, HEIGHT, CELL_SIZE);
  simulake::Grid test_grid(WIDTH / CELL_SIZE, HEIGHT / CELL_SIZE);

  // generate grid
  for (int x = 0; x < HEIGHT / CELL_SIZE; x += 1) {
    for (int y = 0; y < WIDTH / CELL_SIZE; y += 1) {
      test_grid.set_state(x, y, simulake::CellType::AIR);

      if (std::rand() % 2 == 0)
        test_grid.set_state(x, y, simulake::CellType::SAND);
    }
  }

  // render loop
  while (!glfwWindowShouldClose(renderer.get_window().get_window_ptr())) {
    // PROFILE_SCOPE("render loop");

    renderer.render(test_grid);
    test_grid.simulate();

    glfwPollEvents();
  }

  glfwTerminate();
}

void test_simulation() {
  PROFILE_FUNCTION();
  constexpr auto DEBUG_PRINT = true;
  constexpr auto SIM_STEPS = 1000;
  constexpr auto NUM_THREADS = 10;

  // TODO(vir): find a better place for this initialization
  { omp_set_num_threads(NUM_THREADS); }

  // create grid
  const int width = 50, height = 100;
  simulake::Grid grid(width, height);

  // example: demo sand simluation
  {
    // simulake::scope_timer_t timer("full sim");
    PROFILE_SCOPE("sim");

    for (int i = 0; i < SIM_STEPS; i += 1) {
      // spawn new particles in
      grid.set_state(0, (width * 2 / 3) + 1, simulake::CellType::SAND);
      grid.set_state(0, (width * 1 / 3) - 1, simulake::CellType::SAND);

      if constexpr (DEBUG_PRINT) {
        std::cout << grid << std::endl;
        grid.simulate();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
      } else {
        grid.simulate();
      }
    }
  }

  if constexpr (DEBUG_PRINT)
    std::cout << grid << std::endl;
}

int main(int argc, char **argv) {
  // test_simulation();
  test_renderer(argc, argv);
  return 0;
}
