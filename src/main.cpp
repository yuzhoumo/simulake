#include "utils.hpp"
#include <iostream>

#include "grid.hpp"
#include "utils.hpp"

int main() {
  simulake::Grid grid(5, 5);

  // initalize the grid
  grid.set_state(0, 1, simulake::CellType::SAND);
  grid.set_state(0, 2, simulake::CellType::SAND);
  grid.set_state(1, 1, simulake::CellType::SAND);
  grid.set_state(0, 0, simulake::CellType::SAND);
  std::cout << grid << std::endl;

  for (int i = 0; i < 5; i += 1) {
    grid.simulate();
    std::cout << grid << std::endl;
  }

  return 0;
}
