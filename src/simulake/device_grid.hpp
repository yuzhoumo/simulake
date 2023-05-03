#ifndef SIMULAKE_DEVICE_GRID_HPP
#define SIMULAKE_DEVICE_GRID_HPP

#include <string>
#include <string_view>

#include "simulake.hpp"

#include "cell.hpp"
#include "grid_base.hpp"

namespace simulake {

// TODO(vir): only Release mode should have disabled asserts
#if DEBUG
#define CL_CALL(x) assert(x == CL_SUCCESS)
#else
#define CL_CALL(x)                                                             \
  if (x != CL_SUCCESS) {                                                       \
    ::std::cout << "CL_FAILURE: " << __FILE__ << ':' << __LINE__               \
                << ::std::endl;                                                \
    ::std::abort();                                                            \
  }
#endif

class DeviceGrid : public GridBase {
public:
  /* cell and its attributes in memory */
  struct __attribute__((packed, aligned(8))) device_cell_t {
    CellType type = CellType::NONE;
    float mass = 0.0f;
    bool updated = false;
  };

  /* initialize device grid with empty (AIR) cells */
  explicit DeviceGrid(const std::uint32_t, const std::uint32_t,
                      const std::uint32_t);

  /* enable moves */
  explicit DeviceGrid(DeviceGrid &&) = default;
  DeviceGrid &operator=(DeviceGrid &&) = default;

  /* disable copies */
  explicit DeviceGrid(const DeviceGrid &) = delete;
  DeviceGrid &operator=(const DeviceGrid &) = delete;

  /* release device resources on cleanup */
  ~DeviceGrid();

  /* run simulation step on device and render texture */
  void simulate() noexcept override;

  /* reset grid to empty (AIR) cells */
  void reset() noexcept override;

  /* mouse input api */
  void spawn_cells(const std::tuple<std::uint32_t, std::uint32_t> &,
                   const std::uint32_t,
                   const CellType) noexcept override;

  inline std::uint32_t get_width() const noexcept override { return width; }
  inline std::uint32_t get_height() const noexcept override { return height; }
  constexpr std::uint32_t get_stride() const noexcept override {
    return sizeof(device_cell_t);
  }

  serialized_grid_t serialize() const noexcept override;
  void deserialize(const serialized_grid_t &) noexcept override;

  constexpr bool is_device_grid() const noexcept override {
    return true;
  }

  /* set gl texture target */
  void set_texture_target(const GLuint) noexcept;

  /* useful for testing */
  void initialize_random() const noexcept;
  void print_current() const noexcept;
  void print_both() const noexcept;

private:
  constexpr inline static size_t LOCAL_WIDTH = 10;
  constexpr inline static size_t LOCAL_HEIGHT = 10;

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
    cl_kernel spawn_kernel = nullptr;

    /* buffers */
    cl_mem grid = nullptr;
    cl_mem next_grid = nullptr;
  };

  /* initialize logical device and compute structures */
  void initialize_device() noexcept;
  void initialize_kernels() noexcept;

  /* render into gl texture */
  void render_texture() const noexcept;

  /* helpers */
  static std::string read_program_source(const std::string_view) noexcept;
  void print_cl_debug_info() const noexcept;
  void print_cl_image_debug_info(const cl_image) const noexcept;

  GLuint texture_target;
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
