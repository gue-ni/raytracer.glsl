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

struct Triangle
{
  glm::vec4 v0, v1, v2;
  AABB bounds() const;
  static AABB bounds(const std::vector<Triangle> &ts);
};

struct KdNode
{
  KdNode *left;
  KdNode *right;
  std::vector<uint> primitives;
};

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

private:
  std::vector<KdNode> m_nodes;
};