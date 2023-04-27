#ifndef SIMULAKE_DEVICE_GRID_HPP
#define SIMULAKE_DEVICE_GRID_HPP

#include <string>
#include <string_view>

#include "simulake.hpp"

#include "grid.hpp"

namespace simulake {

// #define CL_CALL(x) assert(x == CL_SUCCESS)
#define CL_CALL(x) if (x != CL_SUCCESS) std::exit(x);

class DeviceGrid {
public:
  /* initialize device grid with empty (AIR) cells */
  explicit DeviceGrid(const std::uint32_t, const std::uint32_t,
                      const std::uint32_t);

  // enable moves
  explicit DeviceGrid(DeviceGrid &&) = default;
  DeviceGrid &operator=(DeviceGrid &&) = default;

  // disable copies
  explicit DeviceGrid(const DeviceGrid &) = delete;
  DeviceGrid &operator=(const DeviceGrid &) = delete;

  /* release device resources on cleanup */
  ~DeviceGrid();

  /* run simulation step on device and render texture */
  void simulate() noexcept;

  /* reset grid to empty (AIR) cells */
  void reset() const noexcept;

  std::vector<CellType> compute_texture() const noexcept;

  /* useful for testing */
  void initialize_random() const noexcept;
  void print_current() const noexcept;
  void print_both() const noexcept;

  inline std::uint32_t get_width() const noexcept { return width; }
  inline std::uint32_t get_height() const noexcept { return height; }
  inline std::uint32_t get_stride() const noexcept { return stride; }

private:
  /* opencl structures */
  struct sim_context_t {
    cl_platform_id platform = nullptr;
    cl_device_id device = nullptr;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;

    /* kernels */
    cl_kernel sim_kernel = nullptr;
    cl_kernel init_kernel = nullptr;
    cl_kernel rand_kernel = nullptr;
    cl_kernel render_kernel = nullptr;

    /* buffers */
    cl_mem grid = nullptr;
    cl_mem next_grid = nullptr;
  };

  /* initialize logical device and compute structures */
  void initialize_device() noexcept;
  void initialize_kernels() noexcept;

  static std::string read_program_source(const std::string_view) noexcept;
  void print_cl_extensions() const noexcept;

  std::uint32_t stride;
  std::uint32_t num_cells;
  std::uint32_t memory_size;
  sim_context_t sim_context;

  bool flip_flag;
  std::uint32_t width;
  std::uint32_t height;
  std::uint32_t cell_size;
};

} // namespace simulake

#endif
