#include <iostream>

#include "callbacks.hpp"
#include "window.hpp"

namespace simulake {

Window::Window(const std::uint32_t _width, const std::uint32_t _height,
               const std::string_view _title)
    : width(_width), height(_height), title(_title) {

  glfwSetErrorCallback(callbacks::error);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);

  _window.reset(
      glfwCreateWindow(width, height, title.data(), nullptr, nullptr));
  if (_window.get() == nullptr) {
    failure_exit();
  }

  glfwMakeContextCurrent(_window.get());

  /* register window event callbacks */
  glfwSetFramebufferSizeCallback(_window.get(), callbacks::framebuffer_size);
  glfwSetKeyCallback(_window.get(), callbacks::key);
  glfwSetCursorEnterCallback(_window.get(), callbacks::cursor_enter);
  glfwSetCursorPosCallback(_window.get(), callbacks::cursor_pos);
  glfwSetScrollCallback(_window.get(), callbacks::scroll);
  glfwSetMouseButtonCallback(_window.get(), callbacks::mouse_button);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    failure_exit();
  }
}

std::tuple<std::uint32_t, std::uint32_t>
Window::get_window_size() const noexcept {
  int width, height;
  // glfwGetFramebufferSize(_window.get(), &width, &height);
  glfwGetWindowSize(_window.get(), &width, &height);
  return std::make_tuple(static_cast<std::uint32_t>(width),
                         static_cast<std::uint32_t>(height));
}

float Window::get_time() const noexcept {
  double time = glfwGetTime();
  return static_cast<float>(time);
}

void Window::poll_events() noexcept { glfwPollEvents(); }

bool Window::should_close() const noexcept {
  return glfwWindowShouldClose(_window.get());
}

void Window::swap_buffers() const noexcept { glfwSwapBuffers(_window.get()); }

[[noreturn]] void Window::failure_exit() const noexcept {
  std::cerr << "ERROR::WINDOW_FAILURE_EXIT" << std::endl;
  glfwTerminate();
  std::exit(-1);
}

GLFWwindow *Window::get_window_ptr() const noexcept { return _window.get(); }

} /* namespace simulake */
