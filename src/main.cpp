#include "cell.hpp"
#include "grid.hpp"
#include <iostream>

int main() {
  simulake::Grid grid(200, 150);
  for (int i = 0; i < 10; i += 1)
    grid.simulate();

  return 0;
}
