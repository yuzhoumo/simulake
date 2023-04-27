#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

namespace simulake {

Shader::Shader(const std::string_view vertex_shader_path,
               const std::string_view fragment_shader_path) {
  _id = build_program(vertex_shader_path.data(), fragment_shader_path.data());
}

GLuint
Shader::build_program(const std::string_view vertex_shader_path,
                      const std::string_view fragment_shader_path) noexcept {
  /* compile shaders */
  GLuint vertex_shader, fragment_shader;
  vertex_shader = compile(vertex_shader_path, ShaderType::VERTEX_SHADER);

  if (vertex_shader == 0) {
    std::cerr << "ERROR::VERTEX_SHADER_COMPILATION_FAILURE" << std::endl;
    return 0;
  }

  fragment_shader = compile(fragment_shader_path, ShaderType::FRAGMENT_SHADER);
  if (fragment_shader == 0) {
    std::cerr << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILURE" << std::endl;
    return 0;
  }

  /* create shader program and link fragment shader */
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  if (check_compile_errors(shader_program, ShaderType::SHADER_PROGRAM) == -1)
    return 0;

  /* clean up shader objects */
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

GLuint Shader::compile(const std::string_view shader_path,
                       const ShaderType type) noexcept {
  // create shader
  GLuint shader = [&type]() {
    if (ShaderType::FRAGMENT_SHADER == type) {
      return glCreateShader(GL_FRAGMENT_SHADER);
    } else if (ShaderType::VERTEX_SHADER == type) {
      return glCreateShader(GL_VERTEX_SHADER);
    }

    assert(false);
    std::exit(0);
  }();

  // read shader source code from path
  std::stringstream stream;
  std::ifstream shader_file{};
  shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    shader_file.open(shader_path);
    stream << shader_file.rdbuf();
    shader_file.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "ERROR::SHADER::FILE_READ_FAILURE" << std::endl;
    assert(false);
  }

  const std::string shader_code = stream.str();

  // compile fragment shader
  const char *shader_code_c_str = shader_code.c_str();
  glShaderSource(shader, 1, &shader_code_c_str, nullptr);
  glCompileShader(shader);

  assert(check_compile_errors(shader, type) != -1);
  return shader;
}

int Shader::check_compile_errors(const GLuint shader,
                                 const ShaderType type) noexcept {
  GLint success{};
  GLchar glfw_log[1024];
  std::string_view error_log;

  if (type == ShaderType::SHADER_PROGRAM) {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(shader, 1024, nullptr, glfw_log);
    error_log = "ERROR::SHADER_PROGRAM_LINKING_ERROR";
  } else {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    glGetShaderInfoLog(shader, 1024, nullptr, glfw_log);
    error_log = "ERROR::SHADER_COMPILATION_ERROR";
  }

  if (success)
    return 0;

  std::cerr << error_log << std::endl;
  std::cerr << glfw_log << std::endl;
  return -1;
}

void Shader::use() const noexcept { glUseProgram(_id); }

std::uint32_t Shader::get_id() const noexcept { return _id; }

void Shader::set_bool(const std::string_view name,
                      const bool value) const noexcept {
  glUniform1i(glGetUniformLocation(_id, name.data()), (int)value);
}

void Shader::set_int(const std::string_view name,
                     const int value) const noexcept {
  glUniform1i(glGetUniformLocation(_id, name.data()), value);
}

void Shader::set_float(const std::string_view name,
                       const float value) const noexcept {
  glUniform1f(glGetUniformLocation(_id, name.data()), value);
}

void Shader::set_float2(const std::string_view name,
                        const glm::vec2 values) const noexcept {
  glUniform2f(glGetUniformLocation(_id, name.data()), values.x, values.y);
}

void Shader::set_float3(const std::string_view name,
                        const glm::vec3 values) const noexcept {
  glUniform3f(glGetUniformLocation(_id, name.data()), values.x, values.y,
              values.z);
}

void Shader::set_float4(const std::string_view name,
                        const glm::vec4 values) const noexcept {
  glUniform4f(glGetUniformLocation(_id, name.data()), values.x, values.y,
              values.z, values.w);
}

void Shader::set_mat4(const std::string_view name,
                      const glm::mat4 value) const noexcept {
  glUniformMatrix4fv(glGetUniformLocation(_id, name.data()), 1, GL_FALSE,
                     glm::value_ptr(value));
}

} // namespace simulake
