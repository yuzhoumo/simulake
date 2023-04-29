#include <filesystem>
#include <iostream>
#include <thread>

#include <omp.h>

#include "application/app.hpp"
#include "simulake/simulake.hpp"

#include "simulake/renderer.hpp"
#include "simulake/shader.hpp"

#include "simulake/device_grid.hpp"
#include "simulake/grid.hpp"
#include "utils.hpp"

void test_renderer() {
  PROFILE_FUNCTION();
  constexpr auto WIDTH = 1920;
  constexpr auto HEIGHT = 1080;
  constexpr auto CELL_SIZE = 1;
  constexpr auto NUM_THREADS = 10;

  {
    int rc = glfwInit();
    assert(rc != 0);

    omp_set_num_threads(NUM_THREADS);
  }

  simulake::Window window{WIDTH, HEIGHT, "simulake"};
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

  // stats
  std::uint64_t frame_count = 0;
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // initial load
  renderer.submit_grid(test_grid);

  // application loop
  while (!glfwWindowShouldClose(window.get_window_ptr())) {
    PROFILE_SCOPE("main_loop");
    frame_count += 1;

    // render step
    {
      // PROFILE_SCOPE("render_pass");
      renderer.render();
      window.swap_buffers();
    }

    // sim step
    {
      // PROFILE_SCOPE("sim_pass");
      test_grid.simulate();
      renderer.submit_grid(test_grid);
    }

    glfwPollEvents();

    // escape key
    if (glfwGetKey(window.get_window_ptr(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get_window_ptr(), GLFW_TRUE);
  }

  // clang-format off
  const auto delta = (std::chrono::high_resolution_clock::now() - start);
  const auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(delta);
  std::cout << "average fps: " << frame_count / duration_s.count() << std::endl;
  // clang-format on

  glfwTerminate();
}

void test_simulation() {
  PROFILE_FUNCTION();
  constexpr auto DEBUG_PRINT = true;
  constexpr auto SIM_STEPS = 1000;
  constexpr auto NUM_THREADS = 10;

  { omp_set_num_threads(NUM_THREADS); }

  // create grid
  const int width = 50, height = 100;
  simulake::Grid grid(width, height);

  // example: demo sand simluation
  {
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

void test_device_grid() {
  PROFILE_FUNCTION();
  constexpr auto WIDTH = 1280;
  constexpr auto HEIGHT = 720;
  constexpr auto CELL_SIZE = 1;

  {
    int rc = glfwInit();
    assert(rc != 0);
  }

  // example: demo sand simluation on gpu
  simulake::Window window = simulake::Window{WIDTH, HEIGHT, "simulake"};
  simulake::Renderer renderer{WIDTH, HEIGHT, CELL_SIZE};
  simulake::DeviceGrid grid(WIDTH / CELL_SIZE, HEIGHT / CELL_SIZE, CELL_SIZE);

  // initialize
  grid.initialize_random();
  renderer.submit_grid(grid);

  // stats
  std::uint64_t frame_count = 0;
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  // application loop
  while (!glfwWindowShouldClose(window.get_window_ptr())) {
    PROFILE_SCOPE("main_loop");

    frame_count += 1;

    // render step
    {
      // PROFILE_SCOPE("render_pass");
      renderer.render();
      window.swap_buffers();
    }

    // sim step
    {
      // PROFILE_SCOPE("sim_pass");
      grid.simulate();
      renderer.submit_grid(grid);
    }

    glfwPollEvents();

    // escape key
    if (glfwGetKey(window.get_window_ptr(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get_window_ptr(), GLFW_TRUE);
  }

  // clang-format off
  const auto delta = (std::chrono::high_resolution_clock::now() - start);
  const auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(delta);
  std::cout << "average fps: " << frame_count / duration_s.count() << std::endl;
  // clang-format on

  glfwTerminate();
}

int main(int argc, char **argv) {
  constexpr auto WIDTH = 1280;
  constexpr auto HEIGHT = 720;
  constexpr auto CELL_SIZE = 4;
  constexpr auto GPU_MODE = false;

  int rc = glfwInit();
  assert(rc != 0);

  simulake::App app = simulake::App{WIDTH, HEIGHT, CELL_SIZE, "simulake"};

  {
    PROFILE_SCOPE("total run time");
    app.run(GPU_MODE);
  }

  // test_simulation();
  // test_renderer();
  // test_device_grid();

  return 0;
}
