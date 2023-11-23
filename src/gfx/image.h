#pragma once

#include <GL/glew.h>
#include <string>
#include <optional>
#include <glm/glm.hpp>

namespace gfx
{
  class Image
  {
  public:
    enum Format : GLint
    {
      RED = GL_RED,
      RG = GL_RG,
      RGB = GL_RGB,
      RGBA = GL_RGBA
    };

    Image() : m_data(nullptr), m_width(0), m_height(0), m_channels(0) {}
    Image(unsigned char *data, int width, int height, int channels)
        : m_data(data), m_width(width), m_height(height), m_channels(channels) {}

    ~Image();

    Image(const Image &other) = delete;
    Image &operator=(const Image &other) = delete;
    Image(Image &&other) noexcept;
    Image &operator=(Image &&other) noexcept;

    static std::optional<Image> open(const std::string &path, bool flip_vertically = false);

    unsigned char *data() const;
    int width() const;
    int height() const;
    int channels() const;
    Format format() const;
    bool write_png(const std::string &path, bool flip_vertically = false) const;
    bool read_png(const std::string &path, bool flip_vertically = false);
    Image crop_image(const glm::ivec2& min, const glm::ivec2& max) const;

  private:
    unsigned char *m_data = nullptr;
    int m_width, m_height, m_channels;
  };
}
