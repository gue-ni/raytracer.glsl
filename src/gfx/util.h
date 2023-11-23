#pragma once

#include <glm/glm.hpp>

#include <cassert>

namespace gfx
{

  // [0, 255] -> [0, 1]
  template <typename T>
  constexpr glm::vec3 rgb(T r, T g, T b)
  {
    return glm::vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)) / 255.0f;
  }

  constexpr glm::vec3 rgb(uint32_t hex)
  {
    assert(hex <= 0xffffffU);
    return rgb((hex & 0xff0000U) >> 16, (hex & 0x00ff00U) >> 8, (hex & 0x0000ffU) >> 0);
  }
}