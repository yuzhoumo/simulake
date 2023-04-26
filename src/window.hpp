#ifndef SIMULAKE_WINDOW_HPP
#define SIMULAKE_WINDOW_HPP

#include <memory>
#include <string_view>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace simulake {

// void framebuffer_size_callback(GLFWwindow *, int, int);

class Window {
public:
  /* initialize window */
  explicit Window(const std::uint32_t, const std::uint32_t,
                  const std::string_view);

  // enable moves
  explicit Window(Window &&);   // move constructor
  Window &operator=(Window &&); // move assignment

  // disable copies
  Window(const Window &) = delete;            // copy constructor
  Window &operator=(const Window &) = delete; // copy assignment

  void swap_buffers() const noexcept;
  GLFWwindow *get_window_ptr() const noexcept;

private:
  /* setup window */
  void setup() noexcept;
  [[noreturn]] void failure_exit() noexcept;

  static void framebuffer_size_callback(GLFWwindow *, int, int);

  // clang-format off
  typedef decltype([](GLFWwindow *ptr) { glfwDestroyWindow(ptr); }) glfw_window_deleter_t;
  typedef std::unique_ptr<GLFWwindow, glfw_window_deleter_t> glfw_window_ptr_t;
  // clang-format on

  glfw_window_ptr_t _window;

  const std::uint32_t width;
  const std::uint32_t height;
  const std::string_view title;
};

} // namespace simulake

#endif
