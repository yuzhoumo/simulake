#include <iostream>
#include <thread>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <omp.h>

#include "grid.hpp"
#include "utils.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

/* initialize GLAD and GLFW, then create window object and set callbacks */
void createGLcontexts(GLFWwindow *&window) {
  if (!glfwInit()) {
    std::cerr << "ERROR::GLFW_INITIALIZATION_FAILURE" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(800, 600, "simulake", nullptr, nullptr);
  if (nullptr == window) {
    std::cerr << "ERROR::GLFW_WINDOW_CREATION_FAILURE" << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(
      window, framebuffer_size_callback); // Set a framebuffer size callback

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "ERROR::GLAD_INITIALIZATION_FAILURE" << std::endl;
    exit(EXIT_FAILURE);
  }

  glViewport(0, 0, 800, 600);
}

void test_renderer() {
  GLFWwindow *window;
  createGLcontexts(window);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
}

void test_simulation() {
  PROFILE_FUNCTION();

  // easy-toggles
  constexpr auto DEBUG_PRINT = false;
  constexpr auto NUM_THREADS = 10;
  constexpr auto SIM_STEPS = 1000;

  // TODO(vir): find a better place for this
  omp_set_num_threads(NUM_THREADS);

  // create grid
  const int width = 500, height = 500;
  simulake::Grid grid(width, height);

  // example: demo sand simluation
  {
    // simulake::scope_timer_t timer("full sim");
    PROFILE_SCOPE("sim");

    for (int i = 0; i < SIM_STEPS; i += 1) {
      // spawn new particles in
      if (i % (height / 2) == 0) {
        if (i % height == 0)
          grid.set_state(0, (width * 2 / 3) + 1, simulake::CellType::SAND);
        else
          grid.set_state(0, (width * 1 / 3) - 1, simulake::CellType::SAND);
      }

      if constexpr (DEBUG_PRINT) {
        std::cout << grid << std::endl;
        grid.simulate();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
      } else {
        grid.simulate();
      }
    }
  }

  if constexpr (DEBUG_PRINT)
    std::cout << grid << std::endl;
}

int main() {
  test_simulation();
  return 0;
}
