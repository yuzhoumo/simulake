#include <iostream>
#include <thread>

#include <omp.h>

#include "grid.hpp"
#include "utils.hpp"

int main() {
  // easy-toggles
  constexpr auto DEBUG_PRINT = false;
  constexpr auto NUM_THREADS = 10;
  constexpr auto SIM_STEPS = 1000;

  // TODO(vir): find a better place for this
  omp_set_num_threads(NUM_THREADS);

  // create grid
  const int width = 1000, height = 1000;
  simulake::Grid grid(width, height);

  // example: demo sand simluation
  {
    simulake::scope_timer_t timer("full sim");

    for (int i = 0; i < SIM_STEPS; i += 1) {
      // spawn new particles in
      if (i % (height / 2) == 0) {
        if (i % height == 0)
          grid.set_state(0, (width * 2 / 3) + 1, simulake::CellType::SAND);
        else
          grid.set_state(0, (width * 1 / 3) - 1, simulake::CellType::SAND);
      }

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

  return 0;
}
