#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include <vector>
#include <array>
#include <memory>
#include <map>
#include <span>

#include "image.h"

#define DEBUG_OPENGL 0

#if DEBUG_OPENGL
#define GL_CALL(stmt)                          \
  do                                           \
  {                                            \
    stmt;                                      \
    check_gl_error(#stmt, __FILE__, __LINE__); \
  } while (0)
#else
#define GL_CALL(stmt) stmt
#endif

namespace gfx
{

  namespace gl
  {

    void check_gl_error(const char *stmt, const char *fname, int line);

    struct Vertex
    {
      glm::vec3 Position;
      glm::vec3 Normal;
      glm::vec2 TexCoord;
    };

    // abstract opengl object
    struct Object
    {
      Object() = default;
      ~Object() = default;

      Object(const Object &src) = delete;
      Object &operator=(const Object &rhs) = delete;

      Object(Object &&src) noexcept : m_id(src.m_id) { src.m_id = 0; }
      Object &operator=(Object &&rhs) noexcept
      {
        if (this != &rhs)
          std::swap(m_id, rhs.m_id);
        return *this;
      }

      inline operator GLuint() const noexcept { return m_id; };
      inline GLuint id() const { return m_id; }

    protected:
      GLuint m_id = 0;
    };

    struct Buffer : public Object
    {
    public:
      Buffer(GLenum target_) : target(target_) { GL_CALL(glGenBuffers(1, &m_id)); }
      ~Buffer() { GL_CALL(glDeleteBuffers(1, &m_id)); }
      void bind() const { GL_CALL(glBindBuffer(target, m_id)); }
      void unbind() const { GL_CALL(glBindBuffer(target, 0)); }

      template <typename T>
      void buffer_sub_data(size_t offset, const std::span<T> &data)
      {
        GL_CALL(glBufferSubData(offset, data.size_bytes(), data.data()));
      }

      void bind_buffer_range(GLuint index, size_t offset, size_t size)
      {
        GL_CALL(glBindBufferRange(target, index, m_id, offset, size));
      }

      void bind_buffer_base(GLuint index)
      {
        GL_CALL(glBindBufferBase(target, index, m_id));
      }

      template <typename T>
      void buffer_data(const std::span<T> &data, GLenum usage = GL_STATIC_DRAW)
      {
        GL_CALL(glBufferData(target, data.size_bytes(), data.data(), usage));
      }

    protected:
      const GLenum target;
    };

    struct VertexBuffer : public Buffer
    {
      VertexBuffer() : Buffer(GL_ARRAY_BUFFER) {}
    };

    struct ElementBuffer : public Buffer
    {
      ElementBuffer() : Buffer(GL_ELEMENT_ARRAY_BUFFER) {}
    };

    struct UniformBuffer : public Buffer
    {
      UniformBuffer() : Buffer(GL_UNIFORM_BUFFER) {}
    };

    struct ShaderStorageBuffer : public Buffer
    {
      ShaderStorageBuffer() : Buffer(GL_SHADER_STORAGE_BUFFER) {}
    };

    struct FrameBuffer : public Object
    {
      FrameBuffer() { glGenFramebuffers(1, &m_id); }
      ~FrameBuffer() { glDeleteFramebuffers(1, &m_id); }
      void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, m_id); }
      void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
      bool complete() const { return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE; }
    };

    struct VertexArrayObject : public Object
    {
      VertexArrayObject() { glGenVertexArrays(1, &m_id); }
      ~VertexArrayObject() { glDeleteVertexArrays(1, &m_id); }
      void bind() const { glBindVertexArray(m_id); }
      void unbind() const { glBindVertexArray(0); }
    };

    struct RenderBuffer : public Object
    {
      RenderBuffer() { glGenRenderbuffers(1, &m_id); }
      ~RenderBuffer() { glDeleteRenderbuffers(1, &m_id); }
      void bind() const { glBindRenderbuffer(GL_RENDERBUFFER, m_id); }
      void unbind() const { glBindRenderbuffer(GL_RENDERBUFFER, 0); }
    };

    struct ShaderProgram : public Object
    {
      enum ShaderType
      {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_VERTEX_SHADER,
        Compute = GL_COMPUTE_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
      };

      // TODO: search for includes
      struct File
      {
        File(const std::string &path);
        std::string content;
        void preprocess();
      };

      // ShaderProgram() : m_id(glCreateProgram()) {}
      ShaderProgram(const std::map<ShaderType, File> &sources);
      ShaderProgram(const std::string &compute_shader_source);
      ShaderProgram(const std::string &vertex_shader_source, const std::string &fragment_shader_source);
      ~ShaderProgram();
      void bind() const;
      void unbind() const;
      void set_uniform(const std::string &name, GLint value) const;
      void set_uniform(const std::string &name, GLuint value) const;
      void set_uniform(const std::string &name, GLfloat value) const;
      void set_uniform(const std::string &name, const glm::vec3 &value) const;
      void set_uniform(const std::string &name, const glm::vec4 &value) const;
      void set_uniform(const std::string &name, const glm::mat3 &value) const;
      void set_uniform(const std::string &name, const glm::mat4 &value) const;
      void set_uniform_buffer(const std::string &name, GLuint binding = 0U);
      static std::string from_file(const std::string &path);
    };

    struct Texture : public Object
    {
      const GLenum target;

      struct Params
      {
        bool flip_vertically = false;
        GLint wrap = GL_REPEAT;
        GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;
        GLint mag_filter = GL_NEAREST;
      };

      Texture(GLenum target_ = GL_TEXTURE_2D) : target(target_) { glGenTextures(1, &m_id); }
      ~Texture() { glDeleteTextures(1, &m_id); }
      Texture(const Image &image, const Params &params);
      void bind() const;
      void bind(GLuint texture_unit) const;
      void unbind() const;
      void set_parameter(GLenum pname, GLint param);
      void set_parameter(GLenum pname, GLfloat param);
      void set_parameter(GLenum pname, const GLfloat *param);
      static std::shared_ptr<Texture> load(const std::string &path, const Params &params);
      static std::shared_ptr<Texture> load(const std::string &path);
    };

    struct CubemapTexture : public Texture
    {
      CubemapTexture(const std::array<std::string, 6> &paths, bool flip_vertically = false);
    };

    class TextureArray : public Object
    {
    public:
      struct Params
      {
        Image::Format format{Image::Format::RGB};
        glm::ivec2 texture_size{1024};
        int array_size{1};
        GLint wrap_s{GL_CLAMP_TO_BORDER}, wrap_t{GL_CLAMP_TO_BORDER};
        GLint min_filter{GL_LINEAR}, mag_filter{GL_LINEAR};
      };

      TextureArray(const Params &params);
      void bind() const;
      void bind(GLuint texture_unit) const;
      void unbind() const;
      void add_image(const Image &image);
      void set_parameter(GLenum pname, GLint param);
      void set_parameter(GLenum pname, GLfloat param);
      void set_parameter(GLenum pname, const GLfloat *param);

      const GLenum target = GL_TEXTURE_2D_ARRAY;

    private:
      const Image::Format m_format;
      const glm::ivec2 m_texture_size;
      const int m_array_size;
      int m_image_index = 0;
    };
  }
}