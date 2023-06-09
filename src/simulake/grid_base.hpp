#ifndef SIMULAKE_GRIDBASE_HPP
#define SIMULAKE_GRIDBASE_HPP

#include <optional>
#include <vector>

#include "cell.hpp"

namespace simulake {

class GridBase {
public:
  struct serialized_grid_t {
    std::uint32_t width;        /* number of grid columns */
    std::uint32_t height;       /* number of grid rows */
    std::uint32_t stride;       /* number of floats per cell */
    std::vector<float> buffer;  /* 1D buffer of grid data */
  };

  virtual ~GridBase() = default;

  /* iterate the simulation by one step */
  virtual void simulate(float delta_time) noexcept = 0;

  /* reset the grid to air cells (empty) */
  virtual void reset() noexcept = 0;

  /* spawn cells, given grid coords (x,y), radius in cells, and cell type */
  virtual void spawn_cells(const std::tuple<std::uint32_t,
                           std::uint32_t> &center,
                           const std::uint32_t paint_radius,
                           const CellType paint_target) noexcept = 0;

  /* accessor methods for grid dimensions */
  virtual std::uint32_t get_width() const noexcept = 0;
  virtual std::uint32_t get_height() const noexcept = 0;

  /* true if grid type is device grid, false otherwise */
  virtual bool is_device_grid() const noexcept = 0;

  /* accessor method for stride between serialized buffer items */
  virtual std::uint32_t get_stride() const noexcept = 0;

  /* saves grid to float buffer */
  virtual serialized_grid_t serialize() const noexcept = 0;

  /* loads grid from float buffer */
  virtual void deserialize(const serialized_grid_t &data) noexcept = 0;
};

} /* namespace simulake */

#endif
