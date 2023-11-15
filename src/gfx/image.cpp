#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <utility>

namespace gfx
{
  std::optional<Image> Image::open(const std::string& path, bool flip_vertically)
  {
    stbi_set_flip_vertically_on_load(flip_vertically);
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (data != nullptr) {
      return std::optional{ Image(data, width, height, channels) };
    } else {
      return std::nullopt;
    }
  }

  Image::Image(const std::string &path, bool flip_vertically)
  {
    stbi_set_flip_vertically_on_load(flip_vertically);
    m_data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 0);
    if (m_data == nullptr)
    {
      std::cerr << "Failed to open Image: " << path << std::endl;
    }
    assert(m_data != nullptr);
  }

  Image::~Image()
  {
    if (m_data != nullptr)
    {
      stbi_image_free(m_data);
    }
  }

  Image::Image(Image &&other) noexcept
      : m_data(std::exchange(other.m_data, nullptr)), m_width(other.width()), m_height(other.height()), m_channels(other.channels())
  {
  }

  Image &Image::operator=(Image &&other) noexcept
  {
    std::swap(m_data, other.m_data);
    m_width = other.width();
    m_height = other.height();
    m_channels = other.channels();
    return *this;
  }

  unsigned char *Image::data() const { return m_data; }

  int Image::width() const { return m_width; }

  int Image::height() const { return m_height; }

  int Image::channels() const { return m_channels; }

  Image::Format Image::format() const
  {
    assert(1 <= m_channels && m_channels <= 4);
    static Format formats[] = {RED, RG, RGB, RGBA};
    return formats[m_channels - 1];
  }

}
