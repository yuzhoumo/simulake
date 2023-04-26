#ifndef SIMULAKE_SHADER_HPP
#define SIMULAKE_SHADER_HPP

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace simulake {

/* enum for denoting types during shader compilation and error checking */
enum ShaderType : std::uint8_t {
  VERTEX_SHADER = 0,
  FRAGMENT_SHADER,
  SHADER_PROGRAM
};

class Shader {
public:
  explicit Shader() = default;
  explicit Shader(const std::string_view, const std::string_view);

  // enable moves
  explicit Shader(Shader &&) = default;
  Shader &operator=(Shader &&) = default;

  // disable copies
  explicit Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  /* activate shader */
  void use() const noexcept;

  /* get opengl identifier */
  std::uint32_t get_id() const noexcept;

  /* utility functions for setting uniforms */
  void set_bool(const std::string_view, const bool) const noexcept;
  void set_int(const std::string_view, const int) const noexcept;
  void set_float(const std::string_view, const float) const noexcept;
  void set_float2(const std::string_view, const glm::vec2) const noexcept;
  void set_float3(const std::string_view, const glm::vec3) const noexcept;
  void set_float4(const std::string_view, const glm::vec4) const noexcept;
  void set_mat4(const std::string_view, const glm::mat4) const noexcept;

private:
  /* opengl shader identifier */
  std::uint32_t _id;

  /* create shader program from paths of the fragment shader and vertex
   * shader source GLSL files */
  static GLuint build_program(const std::string_view,
                              const std::string_view) noexcept;

  /* compile shader of type LOADER_TYPE_VERT_SHADER or LOADER_TYPE_FRAG_SHADER
   * given the path to the shader source GLSL file */
  static GLuint compile(const std::string_view, const ShaderType) noexcept;

  /* private method for checking shader compilation errors */
  static int check_compile_errors(const GLuint, const ShaderType) noexcept;
};

} // namespace simulake
#endif
