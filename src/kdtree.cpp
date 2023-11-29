#include "kdtree.h"

// https://computergraphics.stackexchange.com/questions/7828/difference-between-bvh-and-octree-k-d-trees
KdTree::KdTree(const std::vector<glm::vec4> vertices)
{
  assert(vertices.size() % 3 == 0);

  std::vector<Triangle> triangles;

  for (int i = 0; i < vertices.size() / 3; i++)
  {
    triangles.push_back({vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2]});
  }

  std::vector<AABB> bbs; // boudning boxes

  for (const Triangle &triangle : triangles)
  {
    bbs.push_back(triangle.bounds());
  }

  Node root;
  root.bounds = AABB::bounds(bbs);
  root.children.insert(root.children.end(), bbs.begin(), bbs.end());

  m_nodes.push_back(root);

  uint max_subdivisions = 5;
  uint subdivisions = 0;

  while (true)
  {
  }
}

AABB Triangle::bounds() const
{
  AABB aabb;
  aabb.min = glm::min(v0, glm::min(v1, v2));
  aabb.max = glm::max(v0, glm::max(v1, v2));
  return aabb;
}

AABB Triangle::bounds(const std::vector<Triangle> &ts)
{
  AABB aabb;
  for (const auto &t : ts)
  {
    AABB bounds = t.bounds();
    aabb.min = glm::min(aabb.min, bounds.min);
    aabb.max = glm::max(aabb.max, bounds.max);
  }
  return aabb;
}

AABB AABB::bounds(const std::vector<AABB> &boxes)
{
  AABB aabb;
  for (const auto &bounds : boxes)
  {
    aabb.min = glm::min(aabb.min, bounds.min);
    aabb.max = glm::max(aabb.max, bounds.max);
  }
  return aabb;
}

bool AABB::overlaps(const AABB &other) const
{
  return false;
}

bool AABB::contains(const AABB &other) const
{
  return false;
}
