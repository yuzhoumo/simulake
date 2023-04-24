#pragma once

#include "cell.hpp"
#include <memory>

/* represents the CA-Grid */
class Grid {
public:
  /* initialize an empty grid filled with AIR */
  Grid(const uint32_t width, const uint32_t height); // default constructor
  Grid(const Grid &other);                           // copy constructor
  Grid(Grid &&other);                                // move constructor
  Grid &operator=(const Grid &other);                // copy assignment operator
  Grid &operator=(Grid &&other);                     // move assignment operator

  /* dealloc memory on destruction */
  ~Grid();

  /* reset to empty grid */
  void reset() noexcept;

  /* simulate the next step */
  void simulate() noexcept;

private:
  CellType **_grid;
  uint32_t width, height;
};
