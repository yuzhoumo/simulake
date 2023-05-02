#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include "loader.hpp"

namespace simulake {

GridBase::serialized_grid_t Loader::load_grid(const std::string_view path) {
  std::filesystem::path file_path(path);

  if (!std::filesystem::exists(file_path)) {
    throw std::runtime_error("File does not exist.");
  }

  std::ifstream input_file(file_path, std::ios::binary);

  if (!input_file) {
    throw std::runtime_error("Failed to open file for reading.");
  }

  GridBase::serialized_grid_t grid;

  input_file.read(reinterpret_cast<char*>(&grid.width), sizeof(grid.width));
  input_file.read(reinterpret_cast<char*>(&grid.height), sizeof(grid.height));
  input_file.read(reinterpret_cast<char*>(&grid.stride), sizeof(grid.stride));

  std::size_t buffer_size = grid.width * grid.height * grid.stride;
  grid.buffer.resize(buffer_size);
  input_file.read(reinterpret_cast<char*>(grid.buffer.data()),
                                          buffer_size * sizeof(float));

  if (!input_file) {
    throw std::runtime_error("Failed to read data from file.");
  }

  input_file.close();

  return grid;
}

void Loader::store_grid(const GridBase::serialized_grid_t &data,
                const std::string_view path) {

  std::filesystem::path file_path(path);

  if (file_path.empty()) {
    /* get time */
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                now.time_since_epoch()) % 1000;
    /* format time */
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_time_t), "%Y%m%d%H%M%S");
    oss << '_' << std::setfill('0') << std::setw(3) << now_ms.count();

    /* create base file path */
    std::string file_path_base = std::to_string(data.width) +
                  "x" + std::to_string(data.height) +
                  "x" + std::to_string(data.stride) +
                  "_" + oss.str();

    /* append count if file already exists */
    int counter = 0;
    do {
      file_path = std::filesystem::path(file_path_base +
                  (counter > 0 ? "_" + std::to_string(counter) : "") + ".dat");
      counter++;
    } while (std::filesystem::exists(file_path));
  }

  /* write binary file */
  std::ofstream output_file(file_path, std::ios::binary);

  if (!output_file) {
    throw std::runtime_error("Failed to open file for writing.");
  }

  output_file.write(reinterpret_cast<const char*>(&data.width), sizeof(data.width));
  output_file.write(reinterpret_cast<const char*>(&data.height), sizeof(data.height));
  output_file.write(reinterpret_cast<const char*>(&data.stride), sizeof(data.stride));

  output_file.write(reinterpret_cast<const char*>(data.buffer.data()),
                       data.width * data.height * data.stride * sizeof(float));

  if (!output_file) {
    throw std::runtime_error("Failed to write data to file.");
  }

  output_file.close();
}

} /* namespace simulake */
