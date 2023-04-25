#include <filesystem>
#include <iostream>
#include <thread>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <omp.h>

#include "constants.hpp"
#include "shader.hpp"

#include "grid.hpp"
#include "utils.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

/* initialize GLAD and GLFW, then create window object and set callbacks */
void create_gl_contexts(GLFWwindow *&window) {
  if (!glfwInit()) {
    std::cerr << "ERROR::GLFW_INITIALIZATION_FAILURE" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT,
                            WINDOW_TITLE, nullptr, nullptr);
  if (nullptr == window) {
    std::cerr << "ERROR::GLFW_WINDOW_CREATION_FAILURE" << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "ERROR::GLAD_INITIALIZATION_FAILURE" << std::endl;
    exit(EXIT_FAILURE);
  }

  glViewport(0, 0, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
}

/* clean up objects and exit */
void cleanup(GLFWwindow *&window, int exit_code) {
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

void test_renderer(int argc, char **argv) {
  GLFWwindow *window;
  create_gl_contexts(window);

  if (3 != argc) {
    std::cerr << "ERROR::INCORRECT_ARG_COUNT: " << argc << ", expected 3."
              << std::endl;
    exit(EXIT_FAILURE);
  }

  /* load and compile shaders */
  const auto vert_shader_path = std::filesystem::absolute(VERTEX_SHADER_PATH);
  const auto frag_shader_path = std::filesystem::absolute(FRAGMENT_SHADER_PATH);
  const Shader shader{vert_shader_path.string(), frag_shader_path.string()};

  if (0 == shader.get_id()) {
    std::cerr << "ERROR::SHADER_CREATION_FAILURE" << std::endl;
    cleanup(window, EXIT_FAILURE);
  }

  /* load fullscreen quad */
  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(FS_QUAD), FS_QUAD, GL_STATIC_DRAW);

  glVertexAttribPointer(FS_QUAD_VERTEX_ATTRIB_PARAMS);
  glEnableVertexAttribArray(FS_QUAD_VERTEX_ATTRIB_INDEX);

  // render loop
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();

    /* draw frame */
    glBindVertexArray(VAO);
    glDrawArrays(FS_QUAD_DRAW_ARRAY_PARAMS);
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

int main(int argc, char **argv) {
  test_simulation();
  // test_renderer(argc, argv);
  return 0;
}
