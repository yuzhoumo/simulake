#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "device_grid.hpp"

namespace simulake {
DeviceGrid::DeviceGrid(const std::uint32_t _width, const std::uint32_t _height,
                       const std::uint32_t _cell_size)
    : flip_flag(true), width(_width), height(_height), cell_size(_cell_size) {
  num_cells = width * height;
  memory_size = num_cells * sizeof(device_cell_t);

  initialize_device();
  initialize_kernels();
  reset();
}

DeviceGrid::~DeviceGrid() {
  CL_CALL(clReleaseMemObject(sim_context.grid));
  CL_CALL(clReleaseMemObject(sim_context.next_grid));
  CL_CALL(clReleaseKernel(sim_context.init_kernel));
  CL_CALL(clReleaseKernel(sim_context.sim_kernel));
  CL_CALL(clReleaseKernel(sim_context.rand_kernel));
  CL_CALL(clReleaseKernel(sim_context.render_kernel));
  CL_CALL(clReleaseKernel(sim_context.spawn_kernel));
  CL_CALL(clReleaseProgram(sim_context.program));
  CL_CALL(clReleaseCommandQueue(sim_context.queue));
  CL_CALL(clReleaseContext(sim_context.context));
}

void DeviceGrid::reset() noexcept {
  // max work group size is 256 = 16 * 16
  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {LOCAL_WIDTH, LOCAL_HEIGHT};

  CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.init_kernel, 2,
                                 nullptr, global_item_size, local_item_size, 0,
                                 nullptr, nullptr));

  // wait for kernel to finish
  CL_CALL(clFinish(sim_context.queue));
}

void DeviceGrid::initialize_random() const noexcept {
  // max work group size is 256 = 16 * 16
  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {LOCAL_WIDTH, LOCAL_HEIGHT};

  CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.rand_kernel, 2,
                                 nullptr, global_item_size, local_item_size, 0,
                                 nullptr, nullptr));

  // wait for kernel to finish
  CL_CALL(clFinish(sim_context.queue));
}

void DeviceGrid::simulate() noexcept {
  // max work group size is 256 = 16 * 16
  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {LOCAL_WIDTH, LOCAL_HEIGHT};

  // simulation step
  {
    // clang-format off
    CL_CALL(clSetKernelArg(sim_context.sim_kernel, flip_flag ? 0 : 1, sizeof(cl_mem), &sim_context.grid));
    CL_CALL(clSetKernelArg(sim_context.sim_kernel, flip_flag ? 1 : 0, sizeof(cl_mem), &sim_context.next_grid));
    // clang-format on

    CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.sim_kernel, 2,
                                   nullptr, global_item_size, local_item_size,
                                   0, nullptr, nullptr));
    CL_CALL(clEnqueueCopyBuffer(
        sim_context.queue, flip_flag ? sim_context.next_grid : sim_context.grid,
        flip_flag ? sim_context.grid : sim_context.next_grid, 0, 0, memory_size,
        0, nullptr, nullptr));

    render_texture();

    // wait for kernel to finish
    CL_CALL(clFinish(sim_context.queue));
  }

  flip_flag = !flip_flag;
}

GridBase::serialized_grid_t DeviceGrid::serialize() const noexcept {
  std::vector<device_cell_t> grid(num_cells);

  CL_CALL(clEnqueueReadBuffer(
      sim_context.queue, flip_flag ? sim_context.grid : sim_context.next_grid,
      CL_TRUE, 0, memory_size, grid.data(), 0, nullptr, nullptr));
  CL_CALL(clFinish(sim_context.queue));

  // convert in array of floats
  std::vector<float> out_buf(num_cells * NUM_FLOATS, 0.0f);
  for (int i = 0; i < num_cells; i += 1) {
    const auto out_idx = i * NUM_FLOATS;
    out_buf[out_idx + 0] = static_cast<float>(grid[i].type);
    out_buf[out_idx + 1] = static_cast<float>(grid[i].mass);
    out_buf[out_idx + 2] = static_cast<float>(grid[i].velocity.s[0]);
    out_buf[out_idx + 3] = static_cast<float>(grid[i].velocity.s[1]);
  }

  return {get_width(), get_height(), NUM_FLOATS, std::move(out_buf)};
}

void DeviceGrid::deserialize(const GridBase::serialized_grid_t &data) noexcept {
  if (data.width != get_width() || data.height != get_height() ||
      data.stride != NUM_FLOATS) {
    std::cerr << "NOT_IMPLEMENTED::DEVICE_GRID::DESERIALIZE: buffer resize"
              << std::endl;
    std::exit(-1);
    return;
  }

  std::vector<device_cell_t> grid(num_cells);
  const auto height = get_height();
  const auto width = get_width();

  for (int idx = 0, cell = 0; idx < data.buffer.size(); idx += NUM_FLOATS) {
    const auto in_idx = idx / NUM_FLOATS;
    grid[in_idx].type = static_cast<CellType>(data.buffer[idx + 0]);
    grid[in_idx].mass = static_cast<float>(data.buffer[idx + 1]);
    grid[in_idx].velocity.s[0] = static_cast<cl_float>(data.buffer[idx + 2]);
    grid[in_idx].velocity.s[1] = static_cast<cl_float>(data.buffer[idx + 3]);
  }

  CL_CALL(clEnqueueWriteBuffer(sim_context.queue, sim_context.grid, CL_TRUE, 0,
                               memory_size, grid.data(), 0, nullptr, nullptr));
  CL_CALL(clEnqueueCopyBuffer(sim_context.queue, sim_context.grid,
                              sim_context.next_grid, 0, 0, memory_size, 0,
                              nullptr, nullptr));
}

void DeviceGrid::render_texture() const noexcept {
  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {LOCAL_WIDTH, LOCAL_HEIGHT};

  // NOTE(vir):
  // - image is CL_MEM_OBJECT_IMAGE2D;
  // - image format and datatype what texture was initialized to
  // - eg: with stride = 2, format = CL_RG (2 channels), type = CL_FLOAT
  cl_int error = CL_SUCCESS;
  cl_image image =
      clCreateFromGLTexture(sim_context.context, CL_MEM_WRITE_ONLY,
                            GL_TEXTURE_2D, 0, texture_target, &error);
  CL_CALL(error);

#if DEBUG
  // show only once
  const static bool _ = [this, &image]() {
    print_cl_image_debug_info(image);
    return true;
  }();
#endif

  // NOTE(vir): no need to call clEnqueueAcquireGLObjects

  // clang-format off
  // TODO(vir): do we need both current and previous frame? might be useful for effects
  CL_CALL(clSetKernelArg(sim_context.render_kernel, 0, sizeof(cl_image), &image));
  CL_CALL(clSetKernelArg(sim_context.render_kernel, flip_flag ? 2 : 1, sizeof(cl_mem), &sim_context.grid));
  CL_CALL(clSetKernelArg(sim_context.render_kernel, flip_flag ? 1 : 2, sizeof(cl_mem), &sim_context.next_grid));
  // clang-format on

  // render into texture
  CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.render_kernel,
                                 2, nullptr, global_item_size, local_item_size,
                                 0, nullptr, nullptr));
}

void DeviceGrid::set_texture_target(const GLuint target) noexcept {
  texture_target = target;
}

void DeviceGrid::spawn_cells(
    const std::tuple<std::uint32_t, std::uint32_t> &center,
    const std::uint32_t paint_radius, const CellType paint_target) noexcept {

  const size_t global_item_size[] = {width, height};
  const size_t local_item_size[] = {LOCAL_WIDTH, LOCAL_HEIGHT};
  const auto radius = static_cast<unsigned int>(paint_radius);
  const auto target = static_cast<unsigned int>(paint_target);
  const cl_uint2 grid_xy = {static_cast<unsigned int>(std::get<0>(center)),
                            static_cast<unsigned int>(std::get<1>(center))};

  // update the last rendered grid, do not overwrite existing non-vacant cells
  // clang-format off
  CL_CALL(clSetKernelArg(sim_context.spawn_kernel, flip_flag ? 0 : 1, sizeof(cl_mem), &sim_context.grid));
  CL_CALL(clSetKernelArg(sim_context.spawn_kernel, flip_flag ? 1 : 0, sizeof(cl_mem), &sim_context.next_grid));
  CL_CALL(clSetKernelArg(sim_context.spawn_kernel, 2, sizeof(cl_uint2), &grid_xy));
  CL_CALL(clSetKernelArg(sim_context.spawn_kernel, 3, sizeof(unsigned int), &radius));
  CL_CALL(clSetKernelArg(sim_context.spawn_kernel, 4, sizeof(unsigned int), &target));
  // clang-format on

  CL_CALL(clEnqueueNDRangeKernel(sim_context.queue, sim_context.spawn_kernel, 2,
                                 nullptr, global_item_size, local_item_size, 0,
                                 nullptr, nullptr));
}

void DeviceGrid::print_current() const noexcept {
  std::vector<device_cell_t> grid(num_cells);

  CL_CALL(clEnqueueReadBuffer(
      sim_context.queue, flip_flag ? sim_context.grid : sim_context.next_grid,
      CL_TRUE, 0, memory_size, grid.data(), 0, nullptr, nullptr));
  CL_CALL(clFinish(sim_context.queue));

  for (int row = 0; row < height; row += 1) {
    for (int col = 0; col < width; col += 1) {
      const auto index = row * width + col;
      std::cout << grid[index].type;
    }
    std::cout << '\n';
  }

  std::cout << "---\n";
}

void DeviceGrid::print_both() const noexcept {
  std::vector<device_cell_t> grid(num_cells);
  std::vector<device_cell_t> next_grid(num_cells);

  // copy data device -> cpu
  // clang-format off
  CL_CALL(clEnqueueReadBuffer(sim_context.queue, sim_context.grid, CL_TRUE, 0, memory_size, grid.data(), 0, nullptr, nullptr));
  CL_CALL(clEnqueueReadBuffer(sim_context.queue, sim_context.next_grid, CL_TRUE, 0, memory_size, next_grid.data(), 0, nullptr, nullptr));
  // clang-format on

  CL_CALL(clFinish(sim_context.queue));

  for (int row = 0; row < height; row += 1) {
    for (int col = 0; col < width; col += 1) {
      const auto index = row * width + col;
      std::cout << grid[index].type;
    }

    std::cout << '|';

    for (int col = 0; col < width; col += 1) {
      const auto index = row * width + col;
      std::cout << next_grid[index].type;
    }

    std::cout << '\n';
  }

  std::cout << "---\n";
}

void DeviceGrid::print_cl_debug_info() const noexcept {
  cl_uint max_compute_units;
  cl_ulong max_mem_alloc_size;
  size_t max_work_group_size;

  // clang-format off
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units), &max_compute_units, nullptr));
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, nullptr));
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, nullptr));

  cl_uint max_work_item_dims;
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(max_work_item_dims), &max_work_item_dims, NULL));

  size_t work_item_sizes[max_work_item_dims];
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(work_item_sizes), &work_item_sizes, NULL));

  size_t extension_size = 0;
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_EXTENSIONS, 0, NULL, &extension_size));

  char *extensions = new char[extension_size];
  CL_CALL(clGetDeviceInfo(sim_context.device, CL_DEVICE_EXTENSIONS, extension_size, extensions, NULL));
  // clang-format on

  std::cout << "---OPENCL DEVICE INFO---";
  std::cout << "\nmax_mem_alloc_size(mb): " << max_mem_alloc_size / 1024 / 1024;
  std::cout << "\nmax_compute_units:      " << max_compute_units;
  std::cout << "\nmax_work_group_size:    " << max_work_group_size;
  std::cout << "\nmax_work_item_dims:     " << max_work_item_dims;
  std::cout << "\nmax_work_item_sizes:    " << work_item_sizes[0];
  std::cout << "\n------------------------";

  std::cout << "\n---OPENCL EXTENSIONS---\n";
  std::cout << extensions;
  std::cout << "\n-----------------------" << std::endl;

  delete[] extensions;
}

void DeviceGrid::print_cl_image_debug_info(
    const cl_image image) const noexcept {
  cl_int error = CL_SUCCESS;

  cl_image_format format{};
  size_t image_width = 0;
  size_t image_height = 0;
  size_t image_depth = 0;
  size_t elem_size = 0;

  // clang-format off
  CL_CALL(clGetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &format, nullptr));
  CL_CALL(clGetImageInfo(image, CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &elem_size, nullptr));
  CL_CALL(clGetImageInfo(image, CL_IMAGE_WIDTH, sizeof(size_t), &image_width, nullptr));
  CL_CALL(clGetImageInfo(image, CL_IMAGE_HEIGHT, sizeof(size_t), &image_height, nullptr));
  CL_CALL(clGetImageInfo(image, CL_IMAGE_DEPTH, sizeof(size_t), &image_depth, nullptr));
  // clang-format on

  std::cout << "---OPENCL-OPENGL texture-cl_iamge---";
  std::cout << "\nimage_channel_order:     " << format.image_channel_order;
  std::cout << "\nimage_channel_data_type: " << format.image_channel_data_type;
  std::cout << "\nelem_size (bytes):       " << elem_size;
  std::cout << "\nimage_width:             " << image_width;
  std::cout << "\nimage_height:            " << image_height;
  std::cout << "\nimage_depth:             " << image_depth;
  std::cout << "\n------------------------------------" << std::endl;
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

void DeviceGrid::initialize_device() noexcept {
  cl_int error = CL_SUCCESS;

  // create platform and logical device
  CL_CALL(clGetPlatformIDs(1, &sim_context.platform, nullptr));
  CL_CALL(clGetDeviceIDs(sim_context.platform, CL_DEVICE_TYPE_GPU, 1,
                         &sim_context.device, nullptr));

#if DEBUG
  print_cl_debug_info();
#endif

#ifdef __APPLE__
  // NOTE(vir): opengl opencl interop
  CGLContextObj cgl_context = CGLGetCurrentContext();
  if (cgl_context != nullptr) {

    // clang-format off
    CGLShareGroupObj share_group = CGLGetShareGroup(cgl_context);
    cl_context_properties properties[] = { CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)share_group, 0};
    // clang-format on

    gcl_gl_set_sharegroup(share_group);
    sim_context.context = clCreateContext(properties, 1, &sim_context.device,
                                          nullptr, nullptr, &error);
    CL_CALL(error);

    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "OPENGL-OPENCL sharegroup SUCCESSFULLY created" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
  } else {

#endif

    // create context
    sim_context.context =
        clCreateContext(0, 1, &sim_context.device, nullptr, nullptr, &error);
    CL_CALL(error);

    std::cout << "------------------------------------" << std::endl;
    std::cout << "OPENGL-OPENCL sharegroup NOT created" << std::endl;
    std::cout << "------------------------------------" << std::endl;

#ifdef __APPLE__
  }
#endif

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
  constexpr auto RENDER_KERNEL_NAME = "render_texture";
  constexpr auto SPAWN_KERNEL_NAME = "spawn_cells";

  const auto kernel_source = read_program_source(PROGRAM_PATH);
  const char *kernel_source_cstr = kernel_source.c_str();
  const auto kernel_source_size = kernel_source.size();

  // clang-format off
  cl_int error = CL_SUCCESS;

  // allocate opencl buffers
  {
    // double buffering, need this to be read/write for loader
    sim_context.grid = clCreateBuffer(sim_context.context, CL_MEM_READ_WRITE, memory_size, nullptr, &error);
    CL_CALL(error);

    sim_context.next_grid = clCreateBuffer(sim_context.context, CL_MEM_HOST_READ_ONLY, memory_size, nullptr, &error);
    CL_CALL(error);
  }

  // create and compile program
  sim_context.program = clCreateProgramWithSource(sim_context.context, 1, &kernel_source_cstr, &kernel_source_size, &error);
  CL_CALL(error);

  if (clBuildProgram(sim_context.program, 0, nullptr, nullptr, nullptr, nullptr) != CL_SUCCESS) {
    // get the build log from the device
    cl_device_id deviceId;
    size_t buildLogSize;
    CL_CALL(clGetContextInfo(sim_context.context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &deviceId, nullptr));
    CL_CALL(clGetProgramBuildInfo(sim_context.program, deviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &buildLogSize));

    std::vector<char> buildLog(buildLogSize);
    CL_CALL(clGetProgramBuildInfo(sim_context.program, deviceId, CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog.data(), nullptr));

    // print the build log to std::cout
    std::cout << "OpenCL build log:\n" << buildLog.data() << std::endl;
  }

  // simulation kernel
  sim_context.sim_kernel = clCreateKernel(sim_context.program, SIM_KERNEL_NAME, &error);
  CL_CALL(error);

  // initialization kernel
  sim_context.init_kernel = clCreateKernel(sim_context.program, INIT_KERNEL_NAME, &error);
  CL_CALL(error);

  // random initialization kernel
  sim_context.rand_kernel = clCreateKernel(sim_context.program, RAND_KERNEL_NAME, &error);
  CL_CALL(error);

  // texture compute kernel
  sim_context.render_kernel = clCreateKernel(sim_context.program, RENDER_KERNEL_NAME, &error);
  CL_CALL(error);

  // mouse input kernel
  sim_context.spawn_kernel = clCreateKernel(sim_context.program, SPAWN_KERNEL_NAME, &error);
  CL_CALL(error);

  cl_uint2 grid_dim = {width, height};

  // set init kernel args: fixed
  CL_CALL(clSetKernelArg(sim_context.init_kernel, 0, sizeof(cl_mem), &sim_context.grid));
  CL_CALL(clSetKernelArg(sim_context.init_kernel, 1, sizeof(cl_mem), &sim_context.next_grid));
  CL_CALL(clSetKernelArg(sim_context.init_kernel, 2, sizeof(cl_uint2), &grid_dim));

  // set random init kernel args: fixed
  CL_CALL(clSetKernelArg(sim_context.rand_kernel, 0, sizeof(cl_mem), &sim_context.grid));
  CL_CALL(clSetKernelArg(sim_context.rand_kernel, 1, sizeof(cl_mem), &sim_context.next_grid));
  CL_CALL(clSetKernelArg(sim_context.rand_kernel, 2, sizeof(cl_uint2), &grid_dim));

  // NOTE(vir): we set render kernel data args in Device::render_texture()
  // these are the fixed ones
  CL_CALL(clSetKernelArg(sim_context.render_kernel, 3, sizeof(cl_uint2), &grid_dim));
  CL_CALL(clSetKernelArg(sim_context.render_kernel, 4, sizeof(unsigned int), &cell_size));

  // NOTE(vir): we set spawn kernel data args in DeviceGrid::spawn_cells()
  // these are the fixed ones
  CL_CALL(clSetKernelArg(sim_context.spawn_kernel, 5, sizeof(cl_uint2), &grid_dim));
  CL_CALL(clSetKernelArg(sim_context.spawn_kernel, 6, sizeof(unsigned int), &cell_size));

  // NOTE(vir): we set sim kernel data args in DeviceGrid::simulate()
  // these are the fixed ones
  CL_CALL(clSetKernelArg(sim_context.sim_kernel, 2, sizeof(cl_uint2), &grid_dim));
  // clang-format on
}

} // namespace simulake
