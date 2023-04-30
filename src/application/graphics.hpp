#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace simulake {

inline void init_window_context() {
  int success = glfwInit() != 0;
  assert(success);
}

} /* namespace simulake */

#endif
