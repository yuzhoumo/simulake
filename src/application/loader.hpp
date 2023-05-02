#ifndef APP_LOADER_HPP
#define APP_LOADER_HPP

#include "../simulake/grid_base.hpp"

namespace simulake {

class Loader {
public:
  static GridBase::serialized_grid_t load_grid(const std::string_view);
  static void store_grid(const GridBase::serialized_grid_t &,
                         const std::string_view = "");
};

} /* namespace simulake */

#endif
