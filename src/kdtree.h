#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <limits>

using uint = unsigned int;

constexpr uint INVALID = std::numeric_limits<uint>::max();

struct AABB
{
  glm::vec4 min;
  glm::vec4 max;
};

struct Triangle 
{
  glm::vec4 v[3];

  AABB bounds() const { 
    return { glm::min(v[0], glm::min(v[1], v[2])), glm::max(v[0], glm::max(v[1], v[2])) };
  }
};

struct KdNode : public AABB
{
  uint left = INVALID;
  uint right = INVALID;
  uint offset = INVALID;
  uint count = 0;
};

inline AABB from_vertices(const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2) {
  return {
    glm::min(v0, glm::min(v1, v2)),
    glm::max(v0, glm::max(v1, v2))
  };
}

inline int triangle_count(const std::vector<glm::vec4> vertices) {
  assert(vertices.size() % 3 == 0);
  return vertices.size() / 3;
}

inline bool intersect(const AABB* a, const AABB* b) {
  return (a->min.x <= b->max.x && a->max.x >= b->min.x) && 
         (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
         (a->min.z <= b->max.z && a->max.z >= b->min.z);
}

inline std::vector<Triangle> to_triangles(const std::vector<glm::vec4> vertices) {
  std::vector<Triangle> triangles;

  for (uint i = 0; i < vertices.size() / 3; i++) {
    Triangle tri;
    tri.v[0] = vertices[i * 3  + 0];
    tri.v[1] = vertices[i * 3  + 1];
    tri.v[2] = vertices[i * 3  + 2];
    triangles.push_back(tri);
  }

  return triangles;
}

// https://en.wikipedia.org/wiki/K-d_tree
// https://en.wikipedia.org/wiki/Binary_space_partitioning
// https://github.com/fogleman/pt/blob/master/pt/tree.go
// https://github.com/ekzhang/rpt/blob/master/src/kdtree.rs
// https://pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Kd-Tree_Accelerator
class KdTree
{
public:
  // every 3 vertices form a single triangle
  // a triangle is considered a primitive
  KdTree(const std::vector<glm::vec4> vertices);

  uint construct(const std::vector<glm::vec4> vertices, const AABB& bounds, int depth = 0);
  static AABB bounds(const std::vector<glm::vec4> vertices);

  std::vector<KdNode> nodes() { return m_nodes; }
  std::vector<glm::vec4> vertices() { return m_vertices; }

private:
  std::vector<KdNode> m_nodes;
  std::vector<glm::vec4> m_vertices;
};
