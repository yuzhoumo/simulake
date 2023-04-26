#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

void Shader::use() const { glUseProgram(_id); }

unsigned int Shader::get_id() const { return _id; }

void Shader::set_bool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(_id, name.c_str()), (int)value);
}

void Shader::set_int(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
}

void Shader::set_float(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(_id, name.c_str()), value);
}

void Shader::set_float2(const std::string &name, glm::vec2 values) const {
  glUniform2f(glGetUniformLocation(_id, name.c_str()), values.x, values.y);
}

void Shader::set_float3(const std::string &name, glm::vec3 values) const {
  glUniform3f(glGetUniformLocation(_id, name.c_str()), values.x, values.y,
              values.z);
}

void Shader::set_float4(const std::string &name, glm::vec4 values) const {
  glUniform4f(glGetUniformLocation(_id, name.c_str()), values.x, values.y,
              values.z, values.w);
}

void Shader::set_mat4(const std::string &name, glm::mat4 value) const {
  glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(value));
}

GLuint Shader::_build_program(const std::string &vertex_shader_path,
                              const std::string &fragment_shader_path) {
  /* compile shaders */
  GLuint vertex_shader, fragment_shader;
  vertex_shader = _compile(vertex_shader_path, ShaderType::VERTEX_SHADER);

  if (0 == vertex_shader) {
    std::cerr << "ERROR::VERTEX_SHADER_COMPILATION_FAILURE" << std::endl;
    return 0;
  }

  fragment_shader = _compile(fragment_shader_path, ShaderType::FRAGMENT_SHADER);
  if (0 == fragment_shader) {
    std::cerr << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILURE" << std::endl;
    return 0;
  }

  /* create shader program and link fragment shader */
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  if (-1 == _check_compile_errors(shader_program, ShaderType::SHADER_PROGRAM))
    return 0;

  /* clean up shader objects */
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

GLuint Shader::_compile(const std::string &shader_path, const ShaderType type) {
  /* create shader */
  GLuint shader;
  if (ShaderType::FRAGMENT_SHADER == type) {
    shader = glCreateShader(GL_FRAGMENT_SHADER);
  } else if (ShaderType::VERTEX_SHADER == type) {
    shader = glCreateShader(GL_VERTEX_SHADER);
  } else {
    std::cerr << "ERROR::SHADER_TYPE_ERROR Invalid shader type: " << type
              << std::endl;
    return 0;
  }

  /* read shader source code from path */
  std::string shader_code;
  std::ifstream shader_file;
  shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    std::stringstream stream;
    shader_file.open(shader_path);
    stream << shader_file.rdbuf();
    shader_file.close();
    shader_code = stream.str();
  } catch (std::ifstream::failure e) {
    std::cerr << "ERROR::SHADER::FILE_READ_FAILURE" << std::endl;
  }

  /* compile fragment shader */
  const char *shader_code_c_str = shader_code.c_str();
  glShaderSource(shader, 1, &shader_code_c_str, nullptr);
  glCompileShader(shader);
  if (_check_compile_errors(shader, type) == -1)
    return 0;

  return shader;
}

int Shader::_check_compile_errors(const GLuint shader, const ShaderType type) {
  GLint success;
  GLchar info_log[1024];
  std::string err_msg;

  if (ShaderType::SHADER_PROGRAM == type) {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(shader, 1024, nullptr, info_log);
    err_msg = "ERROR::SHADER_PROGRAM_LINKING_ERROR";
  } else {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    glGetShaderInfoLog(shader, 1024, nullptr, info_log);
    err_msg = "ERROR::SHADER_COMPILATION_ERROR";
  }

  if (success)
    return 0;

  std::cerr << info_log << std::endl;
  return -1;
}
