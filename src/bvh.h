#pragma once

#include <glm/glm.hpp>

#include <vector>

using uint = unsigned int;

struct AABB
{
  glm::vec4 min = glm::vec4(-1e5f);
  glm::vec4 max = glm::vec4(+1e5f);
  bool contains(const AABB &other) const;
  bool overlaps(const AABB &other) const;
  static AABB bounds(const std::vector<AABB> &boxes);
};

struct Node
{
  AABB bounds;
  std::vector<Node> children;
  std::vector<uint> primitives;
};

struct Triangle
{
  glm::vec4 v0, v1, v2;
  AABB bounds() const;
  static AABB bounds(const std::vector<Triangle> &ts);
};

// Bounding Volume Hirarchy
// https://en.wikipedia.org/wiki/Binary_space_partitioning
class BVH
{
public:
  // Construct bounding volume hirarchy from a set of triangles
  // every 3 vertices form a single triangle
  // a triangle is considered a primitive
  BVH(const std::vector<glm::vec4> vertices);

private:
  std::vector<Node> m_nodes;
  uint max_primitive_per_volume = 8;
};