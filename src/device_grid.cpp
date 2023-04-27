#include <fstream>
#include <iostream>
#include <sstream>

#include "device_grid.hpp"

namespace simulake {

DeviceGrid::DeviceGrid(const std::uint32_t _width, const std::uint32_t _height)
    : flip_flag(true), width(_width), height(_height) {
  flip_flag = true;
  num_cells = width * height;
  memory_size = num_cells * sizeof(CellType);

  initialize_device();
  initialize_kernels();
  reset();
}

DeviceGrid::~DeviceGrid() {
  cl_int error = CL_SUCCESS;
  CL_CALL(clReleaseMemObject(sim_context.grid));
  CL_CALL(clReleaseMemObject(sim_context.next_grid));
  CL_CALL(clReleaseKernel(sim_context.init_kernel));
  CL_CALL(clReleaseKernel(sim_context.sim_kernel));
  CL_CALL(clReleaseProgram(sim_context.program));
  CL_CALL(clReleaseCommandQueue(sim_context.queue));
  CL_CALL(clReleaseContext(sim_context.context));
}

void DeviceGrid::initialize_device() noexcept {
  cl_int error = CL_SUCCESS;

  // create platform and logical device
  CL_CALL(clGetPlatformIDs(1, &sim_context.platform, nullptr));
  CL_CALL(clGetDeviceIDs(sim_context.platform, CL_DEVICE_TYPE_GPU, 1,
                         &sim_context.device, nullptr));

  // query work group size
  std::pair<cl_device_info, const char *> device_info[] = {
      {CL_DEVICE_MAX_WORK_ITEM_SIZES, "max work item sizes"},
      {CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "max work item dimensions"},
      {CL_DEVICE_MAX_PARAMETER_SIZE, "max parameter size"},
      {CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, "min data type align size"}};

#ifdef DEBUG
  cl_uint max_compute_units;
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_COMPUTE_UNITS,
                          sizeof(max_compute_units), &max_compute_units,
                          nullptr));

  cl_ulong max_mem_alloc_size;
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                          sizeof(max_mem_alloc_size), &max_mem_alloc_size,
                          nullptr));

  size_t max_work_group_size;
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                          sizeof(max_work_group_size), &max_work_group_size,
                          nullptr));

  cl_uint max_work_item_dims;
  CL_CALL(
      clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                      sizeof(max_work_item_dims), &max_work_item_dims, NULL));

  size_t work_item_sizes[max_work_item_dims];
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_WORK_ITEM_SIZES,
                          sizeof(work_item_sizes), &work_item_sizes, NULL));

  std::cout << "---OPENCL DEVICE INFO---" << std::endl;
  std::cout << "max_mem_alloc_size(mb): " << max_mem_alloc_size / 1024 / 1022
            << std::endl;
  std::cout << "max_compute_units:      " << max_compute_units << std::endl;
  std::cout << "max_work_group_size:    " << max_work_group_size << std::endl;
  std::cout << "max_work_item_dims:     " << max_work_item_dims << std::endl;
  std::cout << "max_work_item_sizes:    " << work_item_sizes[0] << std::endl;
  std::cout << "------------------------" << std::endl;
#endif

  // create context
  sim_context.context =
      clCreateContext(0, 1, &sim_context.device, nullptr, nullptr, &error);
  CL_CALL(error);

  // create command queue
  sim_context.queue =
      clCreateCommandQueue(sim_context.context, sim_context.device, 0, &error);
  CL_CALL(error);
}

void DeviceGrid::initialize_kernels() noexcept {
  constexpr auto PROGRAM_PATH = "./shaders/compute.cl";
  constexpr auto SIM_KERNEL_NAME = "simulate";
  constexpr auto INIT_KERNEL_NAME = "initialize";
  constexpr auto RAND_KERNEL_NAME = "random_init";

  const std::string kernel_source = read_program_source(PROGRAM_PATH);
  const char *kernel_source_cstr = kernel_source.c_str();

  // clang-format off
  cl_int error = CL_SUCCESS;

  // allocate grid buffers on device
  sim_context.grid = clCreateBuffer(sim_context.context, CL_MEM_HOST_READ_ONLY, memory_size, nullptr, &error);
  CL_CALL(error);

  sim_context.next_grid = clCreateBuffer(sim_context.context, CL_MEM_HOST_READ_ONLY, memory_size, nullptr, &error);
  CL_CALL(error);

  // create and compile program
  sim_context.program = clCreateProgramWithSource(sim_context.context, 1, &kernel_source_cstr, nullptr, &error);
  CL_CALL(error);

  CL_CALL(clBuildProgram(sim_context.program, 0, nullptr, nullptr, nullptr, nullptr));

  // simulation kernel
  sim_context.sim_kernel = clCreateKernel(sim_context.program, SIM_KERNEL_NAME, &error);
  CL_CALL(error);

  // initialization kernel
  sim_context.init_kernel = clCreateKernel(sim_context.program, INIT_KERNEL_NAME, &error);
  CL_CALL(error);

  // initialization kernel
  sim_context.init_kernel = clCreateKernel(sim_context.program, INIT_KERNEL_NAME, &error);
  CL_CALL(error);

  // random initialization kernel
  sim_context.rand_kernel = clCreateKernel(sim_context.program, RAND_KERNEL_NAME, &error);
  CL_CALL(error);

  // set init kernel args: fixed
  CL_CALL(clSetKernelArg(sim_context.init_kernel, 0, sizeof(cl_mem), &sim_context.grid));
  CL_CALL(clSetKernelArg(sim_context.init_kernel, 1, sizeof(cl_mem), &sim_context.next_grid));
  CL_CALL(clSetKernelArg(sim_context.init_kernel, 2, sizeof(unsigned int), &width));
  CL_CALL(clSetKernelArg(sim_context.init_kernel, 3, sizeof(unsigned int), &height));

  // set random init kernel args: fixed
  CL_CALL(clSetKernelArg(sim_context.rand_kernel, 0, sizeof(cl_mem), &sim_context.grid));
  CL_CALL(clSetKernelArg(sim_context.rand_kernel, 1, sizeof(cl_mem), &sim_context.next_grid));
  CL_CALL(clSetKernelArg(sim_context.rand_kernel, 2, sizeof(unsigned int), &width));
  CL_CALL(clSetKernelArg(sim_context.rand_kernel, 3, sizeof(unsigned int), &height));

  // NOTE(vir): we set sim kernel data args in DeviceGrid::simulate()
  // these two are fixed
  CL_CALL(clSetKernelArg(sim_context.sim_kernel, 2, sizeof(unsigned int), &width));
  CL_CALL(clSetKernelArg(sim_context.sim_kernel, 3, sizeof(unsigned int), &height));

  // clang-format on
}

void DeviceGrid::reset() noexcept {
  // max work group size is 256 = 16 * 16
  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {10, 10};

  CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.init_kernel, 2,
                                 nullptr, global_item_size, local_item_size, 0,
                                 nullptr, nullptr));

  // wait for kernel to finish
  CL_CALL(clFinish(sim_context.queue));

  // test
  std::vector<CellType> grid(num_cells, CellType::NONE);
  std::vector<CellType> next_grid(num_cells, CellType::NONE);
  CL_CALL(clEnqueueReadBuffer(sim_context.queue, sim_context.grid, CL_TRUE, 0,
                              memory_size, grid.data(), 0, nullptr, nullptr));
  CL_CALL(clEnqueueReadBuffer(sim_context.queue, sim_context.next_grid, CL_TRUE,
                              0, memory_size, next_grid.data(), 0, nullptr,
                              nullptr));
  CL_CALL(clFinish(sim_context.queue));

  // make sure init kernel worked kernel
  for (int row = 0; row < height; row += 1) {
    for (int col = 0; col < width; col += 1) {
      const auto index = row * width + col;
      assert(grid[index] == CellType::AIR);
      assert(next_grid[index] == CellType::AIR);
    }
  }
}

void DeviceGrid::simulate() noexcept {
  // clang-format off
  CL_CALL(clSetKernelArg(sim_context.sim_kernel, flip_flag ? 0 : 1, sizeof(cl_mem), &sim_context.grid));
  CL_CALL(clSetKernelArg(sim_context.sim_kernel, flip_flag ? 1 : 0, sizeof(cl_mem), &sim_context.next_grid));
  // clang-format on

  // max work group size is 256 = 16 * 16
  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {10, 10};

  CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.sim_kernel, 2,
                                 nullptr, global_item_size, local_item_size, 0,
                                 nullptr, nullptr));
  CL_CALL(clEnqueueCopyBuffer(
      sim_context.queue, flip_flag ? sim_context.next_grid : sim_context.grid,
      flip_flag ? sim_context.grid : sim_context.next_grid, 0, 0, memory_size,
      0, nullptr, nullptr));

  // wait for kernel to finish
  CL_CALL(clFinish(sim_context.queue));
  flip_flag = !flip_flag;
}

void DeviceGrid::initialize_random() const noexcept {
  // max work group size is 256 = 16 * 16
  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {10, 10};

  CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.rand_kernel, 2,
                                 nullptr, global_item_size, local_item_size, 0,
                                 nullptr, nullptr));

  // wait for kernel to finish
  CL_CALL(clFinish(sim_context.queue));
}

void DeviceGrid::print_current() const noexcept {
  std::vector<CellType> grid(num_cells, CellType::NONE);

  CL_CALL(clEnqueueReadBuffer(
      sim_context.queue, flip_flag ? sim_context.grid : sim_context.next_grid,
      CL_TRUE, 0, memory_size, grid.data(), 0, nullptr, nullptr));
  CL_CALL(clFinish(sim_context.queue));

  // make sure init kernel worked kernel
  for (int row = 0; row < height; row += 1) {
    for (int col = 0; col < width; col += 1) {
      const auto index = row * width + col;
      std::cout << grid[index];
    }
    std::cout << '\n';
  }

  std::cout << "---\n";
}

std::string
DeviceGrid::read_program_source(const std::string_view path) noexcept {
  // read program source code from path
  std::stringstream stream;
  std::ifstream shader_file{};
  shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    shader_file.open(path);
    stream << shader_file.rdbuf();
    shader_file.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "ERROR::PROGRAM::FILE_READ_FAILURE" << std::endl;
    assert(false);
  }

  return stream.str();
}

void DeviceGrid::print_both() const noexcept {
  std::vector<CellType> grid(num_cells, CellType::NONE);
  std::vector<CellType> next_grid(num_cells, CellType::NONE);

  CL_CALL(clEnqueueReadBuffer(sim_context.queue, sim_context.grid, CL_TRUE, 0,
                              memory_size, grid.data(), 0, nullptr, nullptr));
  CL_CALL(clEnqueueReadBuffer(sim_context.queue, sim_context.next_grid, CL_TRUE,
                              0, memory_size, next_grid.data(), 0, nullptr,
                              nullptr));
  CL_CALL(clFinish(sim_context.queue));

  // make sure init kernel worked kernel
  for (int row = 0; row < height; row += 1) {
    for (int col = 0; col < width; col += 1) {
      const auto index = row * width + col;
      std::cout << grid[index];
    }

    std::cout << '|';

    for (int col = 0; col < width; col += 1) {
      const auto index = row * width + col;
      std::cout << next_grid[index];
    }

    std::cout << '\n';
  }

  std::cout << "---\n";
}

} // namespace simulake
