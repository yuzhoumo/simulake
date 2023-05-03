#include <cxxopts.hpp>

#include "application/app.hpp"
#include "application/graphics.hpp"
#include "application/loader.hpp"

#include "test.hpp"

int main(int argc, char *argv[]) {
  std::uint32_t grid_width, grid_height, cell_size;
  std::string grid_file = "";
  bool gpu_mode;

  cxxopts::Options options(argv[0], "A cellular automata physics simulator.\n");

  // clang-format off
  options.add_options()
    ("x,width",      "grid width in cells",     cxxopts::value<std::uint32_t>()->default_value("400"))
    ("y,height",     "grid height in cells",    cxxopts::value<std::uint32_t>()->default_value("200"))
    ("c,cellsize",   "cell size in pixels",     cxxopts::value<std::uint32_t>()->default_value("4"))
    ("g,gpu",        "enable GPU acceleration", cxxopts::value<bool>()->default_value("true"))
    ("l,load",       "load scene from disk",    cxxopts::value<std::string>())
    ("h,help",       "print help");
  // clang-format on

  try {
    auto result = options.parse(argc, argv);
    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(EXIT_SUCCESS);
    }

    if (result.count("load")) {
      grid_file = result["load"].as<std::string>();
    }

    /* set args */
    cell_size = result["cellsize"].as<std::uint32_t>();
    grid_width = result["width"].as<std::uint32_t>();
    grid_height = result["height"].as<std::uint32_t>();
    gpu_mode = result["gpu"].as<bool>();
    std::cout << "gpu_mode: " << gpu_mode << std::endl; /*__DEBUG_PRINT__*/
  } catch (const cxxopts::exceptions::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  /* init and run application */
  simulake::init_window_context();

  /* load grid from disk if path not empty */
  bool load_grid = !grid_file.empty();
  simulake::GridBase::serialized_grid_t data;

  // TODO(vir): width height should not be in file, because they should not be
  if (load_grid) {
    data = simulake::Loader::load_grid(grid_file);
  }

  simulake::App app =
      simulake::App{grid_width, grid_height, cell_size, "simulake"};

  {
    PROFILE_SCOPE("total run time");
    app.run(gpu_mode, load_grid ? &data : nullptr);
  }

  // simulake::test::test_renderer();
  // simulake::test::test_simulation();
  // simulake::test::test_device_grid();

  return 0;
}
