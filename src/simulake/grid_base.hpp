#ifndef SIMULAKE_GRIDBASE_HPP
#define SIMULAKE_GRIDBASE_HPP

#include <optional>
#include <vector>

#include "cell.hpp"

namespace simulake {

class GridBase {
public:
  virtual ~GridBase() = default;

  virtual void simulate() noexcept = 0;
  virtual void reset() noexcept = 0;

  virtual void spawn_cells(const std::tuple<std::uint32_t, std::uint32_t> &,
                           const std::uint32_t,
                           const CellType) noexcept = 0;

  virtual std::uint32_t get_width() const noexcept = 0;
  virtual std::uint32_t get_height() const noexcept = 0;
  virtual std::uint32_t get_stride() const noexcept = 0;

  virtual bool is_device_grid() const noexcept = 0;
};

} /* namespace simulake */

#endif
