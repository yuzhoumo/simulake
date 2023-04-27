#ifndef SIMULAKE_DEVICE_GRID_HPP
#define SIMULAKE_DEVICE_GRID_HPP

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#include <string>
#include <string_view>

#include "grid.hpp"

namespace simulake {

#define CL_CALL(x) assert(x == CL_SUCCESS)

class DeviceGrid {
public:
  /* initialize device grid with empty (AIR) cells */
  explicit DeviceGrid(const std::uint32_t, const std::uint32_t);

  // enable moves
  explicit DeviceGrid(DeviceGrid &&) = default;
  DeviceGrid &operator=(DeviceGrid &&) = default;

  // disable copies
  explicit DeviceGrid(const DeviceGrid &) = delete;
  DeviceGrid &operator=(const DeviceGrid &) = delete;

  /* release device resources on cleanup */
  ~DeviceGrid();

  /* run simulation step on device */
  void simulate() noexcept;

  /* reset grid to empty (AIR) cells */
  void reset() noexcept;

  /* useful for testing */
  void initialize_random() const noexcept;
  void print_current() const noexcept;
  void print_both() const noexcept;

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

    /* buffers */
    cl_mem grid = nullptr;
    cl_mem next_grid = nullptr;
  };

  /* initialize logical device and compute structures */
  void initialize_device() noexcept;
  void initialize_kernels() noexcept;

  static std::string read_program_source(const std::string_view) noexcept;

  std::uint32_t num_cells;
  std::uint32_t memory_size;

  bool flip_flag;
  sim_context_t sim_context;
  std::uint32_t width, height;
};

} // namespace simulake

#endif
