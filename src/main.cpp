#include <cxxopts.hpp>

#include "application/app.hpp"
#include "application/graphics.hpp"
#include "application/loader.hpp"

#include "test.hpp"

int main(int argc, char *argv[]) {
  std::uint32_t width, height, cell_size;
  std::string grid_file = "";
  bool gpu_mode;

  cxxopts::Options options(argv[0], "A cellular automata physics simulator.\n");

  // clang-format off
  options.add_options()
    ("x,width",      "width",                   cxxopts::value<std::uint32_t>()->default_value("800"))
    ("y,height",     "height",                  cxxopts::value<std::uint32_t>()->default_value("600"))
    ("c,cellsize",   "cell size",               cxxopts::value<std::uint32_t>()->default_value("4"))
    ("g,gpu",        "enable GPU acceleration", cxxopts::value<bool>()->default_value("false"))
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
    height = result["height"].as<std::uint32_t>();
    width = result["width"].as<std::uint32_t>();
    gpu_mode = result["gpu"].as<bool>();
  } catch (const cxxopts::exceptions::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  /* init and run application */
  simulake::init_window_context();
  simulake::App app = simulake::App{width, height, cell_size, "simulake"};

  {
    PROFILE_SCOPE("total run time");
    app.run(gpu_mode, grid_file);
  }

  // simulake::test::test_renderer();
  // simulake::test::test_simulation();
  // simulake::test::test_device_grid();

  return 0;
}
