#include "gl.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>


std::string read_file_to_string(const std::string &path)
{
  std::ifstream file(path);
  if (!file.is_open())
  {
    return std::string();
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

namespace gfx
{

  namespace gl
  {

    void check_gl_error(const char *stmt, const char *fname, int line)
    {
      GLenum err = glGetError();
      if (err != GL_NO_ERROR)
      {
        printf("OpenGL error %d, at %s:%i - for %s\n", err, fname, line, stmt);
      }
    }

    ShaderProgram::File::File(const std::string& path) : content(read_file_to_string(path))
    {
      if (content.empty()) {
        std::cerr << "could not read file " << path << std::endl;
      }
      preprocess();
      std::cout << "start ===== " << path << " ======\n";
      std::cout << content << std::endl;
      std::cout << " end ===== " << path << " ======\n";
    }

    void ShaderProgram::File::preprocess()
    {
#if 0
      // TODO: serach and replace header

      std::regex pattern("#include\\s*<([^>]*)>");

      // Search and replace each occurrence of the pattern
      content = std::regex_replace(content, pattern, [&](const std::smatch &match) {
        // Extract the file name from the matched pattern
        std::string filename = match[1].str();

        // Read the content of the included file
        std::ifstream included_file(filename);
        std::string included_content((std::istreambuf_iterator<char>(included_file)), std::istreambuf_iterator<char>());

        return included_content;
    });
#endif
    }

    ShaderProgram::ShaderProgram(const std::string &compute_shader_source)
    {
      int success;
      char log[512];

      const char *shader_source_str = compute_shader_source.c_str();

      GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
      glShaderSource(compute_shader, 1, &shader_source_str, NULL);
      glCompileShader(compute_shader);
      glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
      if (!success)
      {
        glGetShaderInfoLog(compute_shader, 512, NULL, log);
        std::cerr << "Error: " << log;
      }

      m_id = glCreateProgram();
      glAttachShader(m_id, compute_shader);
      glLinkProgram(m_id);
      glGetProgramiv(m_id, GL_LINK_STATUS, &success);
      if (!success)
      {
        glGetProgramInfoLog(m_id, 512, NULL, log);
        std::cerr << "Error: " << log;
      }

      glDeleteShader(compute_shader);
    }

    ShaderProgram::ShaderProgram(const std::string &vertex_shader_source, const std::string &fragment_shader_source)
    {
      int success;
      char log[512];

      const char *vertex_shader_source_str = vertex_shader_source.c_str();

      GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex_shader, 1, &vertex_shader_source_str, NULL);
      glCompileShader(vertex_shader);
      glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
      if (!success)
      {
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        std::cerr << "Error: " << log;
      }

      const char *fragment_shader_source_str = fragment_shader_source.c_str();
      GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment_shader, 1, &fragment_shader_source_str, NULL);
      glCompileShader(fragment_shader);
      glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
      if (!success)
      {
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        std::cerr << "Error: " << log;
      }

      m_id = glCreateProgram();
      glAttachShader(m_id, vertex_shader);
      glAttachShader(m_id, fragment_shader);
      glLinkProgram(m_id);
      glGetProgramiv(m_id, GL_LINK_STATUS, &success);
      if (!success)
      {
        glGetProgramInfoLog(m_id, 512, NULL, log);
        std::cerr << "Error: " << log;
      }

      glDeleteShader(vertex_shader);
      glDeleteShader(fragment_shader);
    }

    ShaderProgram::~ShaderProgram() { glDeleteProgram(m_id); }

    void ShaderProgram::bind() const { glUseProgram(m_id); }

    void ShaderProgram::unbind() const { glUseProgram(0); }

    void ShaderProgram::set_uniform(const std::string &name, GLint value) const
    {
      glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void ShaderProgram::set_uniform(const std::string &name, GLuint value) const
    {
      glUniform1ui(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void ShaderProgram::set_uniform(const std::string &name, GLfloat value) const
    {
      glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void ShaderProgram::set_uniform(const std::string &name, const glm::vec3 &value) const
    {
      glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
    }

    void ShaderProgram::set_uniform(const std::string &name, const glm::vec4 &value) const
    {
      glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
    }

    void ShaderProgram::set_uniform(const std::string &name, const glm::mat3 &value) const
    {
      glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void ShaderProgram::set_uniform(const std::string &name, const glm::mat4 &value) const
    {
      glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void ShaderProgram::set_uniform_buffer(const std::string &name, GLuint binding)
    {
      GLuint index = glGetUniformBlockIndex(m_id, name.c_str());
      glUniformBlockBinding(m_id, index, binding);
    }

    std::string ShaderProgram::string_from_file(const std::string& path)
    {
      std::ifstream file(path);
      if (!file.is_open())
      {
        return std::string();
      }

      std::stringstream buffer;
      buffer << file.rdbuf();
      return buffer.str();
    }

    Texture::Texture(const Image &image, const Params &params) : Texture(GL_TEXTURE_2D)
    {
      glBindTexture(target, m_id);

      set_parameter(GL_TEXTURE_WRAP_S, params.wrap);
      set_parameter(GL_TEXTURE_WRAP_T, params.wrap);
      set_parameter(GL_TEXTURE_MIN_FILTER, params.min_filter);
      set_parameter(GL_TEXTURE_MAG_FILTER, params.mag_filter);

      glTexImage2D(target, 0, image.format(), image.width(), image.height(), 0, image.format(), GL_UNSIGNED_BYTE,
                   image.data());
      glGenerateMipmap(target);
    }

    void Texture::bind(GLuint active_texture) const
    {
      glActiveTexture(GL_TEXTURE0 + active_texture);
      glBindTexture(target, m_id);
    }

    void Texture::bind() const { glBindTexture(target, m_id); }

    void Texture::unbind() const { glBindTexture(target, 0); }

    void Texture::set_parameter(GLenum pname, GLint param) { glTexParameteri(target, pname, param); }

    void Texture::set_parameter(GLenum pname, GLfloat param) { glTexParameterf(target, pname, param); }

    void Texture::set_parameter(GLenum pname, const GLfloat *param) { glTexParameterfv(target, pname, param); }

    std::shared_ptr<Texture> Texture::load(const std::string &path, const Params &params)
    {
      Image image;
      image.read_png(path);
      return std::make_shared<Texture>(image, params);
    }

    std::shared_ptr<Texture> Texture::load(const std::string &path) { return Texture::load(path, {}); }

    CubemapTexture::CubemapTexture(const std::array<std::string, 6> &paths, bool flip_vertically)
        : Texture(GL_TEXTURE_CUBE_MAP)
    {
      bind();

      for (int i = 0; i < 6; i++)
      {
        Image image;
        if (!image.read_png(paths[i], flip_vertically))
        {
          std::cout << "failed to load image " << paths[i] << std::endl;;
        }
        assert(image.data() == nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, image.format(), image.width(), image.height(), 0,
                     image.format(), GL_UNSIGNED_BYTE, image.data());
      }

      set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      set_parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    TextureArray::TextureArray(const Params &params)
        : m_format(params.format), m_texture_size(params.texture_size), m_array_size(params.array_size)
    {
      glBindTexture(target, m_id);

      set_parameter(GL_TEXTURE_WRAP_S, params.wrap_s);
      set_parameter(GL_TEXTURE_WRAP_T, params.wrap_t);
      set_parameter(GL_TEXTURE_MIN_FILTER, params.min_filter);
      set_parameter(GL_TEXTURE_MAG_FILTER, params.mag_filter);

      glTexImage3D(target, 0, m_format, m_texture_size.x, m_texture_size.y, m_array_size, 0, m_format, GL_UNSIGNED_BYTE,
                   NULL);
    }

    void TextureArray::bind() const { glBindTexture(target, m_id); }

    void TextureArray::bind(GLuint texture_unit) const
    {
      glActiveTexture(GL_TEXTURE0 + texture_unit);
      glBindTexture(target, m_id);
    }

    void TextureArray::unbind() const { glBindTexture(target, 0); }

    void TextureArray::set_parameter(GLenum pname, GLint param) { glTexParameteri(target, pname, param); }

    void TextureArray::set_parameter(GLenum pname, GLfloat param) { glTexParameterf(target, pname, param); }

    void TextureArray::set_parameter(GLenum pname, const GLfloat *param) { glTexParameterfv(target, pname, param); }

    void TextureArray::add_image(const Image &image)
    {
      assert(m_format == image.format());
      assert(m_image_index < m_array_size);
      assert(m_texture_size.x == image.width() && m_texture_size.y == image.height());

      glTexSubImage3D(target, 0, 0, 0, m_image_index++, image.width(), image.height(), 1, image.format(), GL_UNSIGNED_BYTE,
                      image.data());
      glGenerateMipmap(target);
    }

  }
}