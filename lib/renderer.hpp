#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "shader.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

struct Cell {
  int type;
  float mass;
};

struct GridData {
  int width;
  int height;
  Cell* cells;
};

class Renderer {
public:
  Renderer(GridData grid_data);

  ~Renderer() {
    glDeleteVertexArrays(1, &_VAO);
    glDeleteBuffers(1, &_VBO);
    glDeleteProgram(_shader.get_id());
  };

  /* disable copy construction/assignment */
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  /* render frame based on current GridData */
  void render(GridData grid_data, GLFWwindow*& window);

private:
  std::vector<std::vector<float>> _generate_chunks(GridData grid_data);

  void _set_cell_size(int cell_size);
  void _set_grid_size(GridData grid_data);
  void _set_viewport_size(int width, int height);
  void _set_viewport_size(glm::ivec2 size);

  Shader _shader;

  glm::ivec2 _grid_size;     /* grid width, height in cells */
  glm::ivec2 _viewport_size; /* viewport width, height in pixels */
  int _num_cells;
  int _cell_size;

  GLuint _VAO, _VBO, _IBO;
};

#endif /* ifndef RENDERER_HPP */
