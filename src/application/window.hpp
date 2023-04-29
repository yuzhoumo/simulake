#ifndef SIMULAKE_WINDOW_HPP
#define SIMULAKE_WINDOW_HPP

#include "graphics.hpp"

#include <memory>
#include <string_view>

namespace simulake {

class Window {
public:
  /* initialize glfw window */
  explicit Window(const std::uint32_t, const std::uint32_t,
                  const std::string_view);

  /* enable moves */
  explicit Window(Window &&) = default;
  Window &operator=(Window &&) = delete;

  /* disable copies */
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  bool should_close() const noexcept;
  void swap_buffers() const noexcept;
  GLFWwindow *get_window_ptr() const noexcept;

private:
  /* print error and terminate */
  [[noreturn]] void failure_exit() const noexcept;

  // clang-format off
  typedef decltype([](GLFWwindow *ptr) { glfwDestroyWindow(ptr); }) glfw_window_deleter_t;
  typedef std::unique_ptr<GLFWwindow, glfw_window_deleter_t> glfw_window_ptr_t;
  // clang-format on

  glfw_window_ptr_t _window;

  const std::uint32_t width;
  const std::uint32_t height;
  const std::string_view title;
};

} /* namespace simulake */

#endif
