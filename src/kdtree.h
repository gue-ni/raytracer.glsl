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

  AABB bounds() const
  {
    return {glm::min(v[0], glm::min(v[1], v[2])), glm::max(v[0], glm::max(v[1], v[2]))};
  }
};

struct KdNode : public AABB
{
  uint left = INVALID;
  uint right = INVALID;
  uint offset = INVALID;
  uint count = 0;
};

inline AABB from_primitives(const glm::vec4 &v0, const glm::vec4 &v1, const glm::vec4 &v2)
{
  return {
      glm::min(v0, glm::min(v1, v2)),
      glm::max(v0, glm::max(v1, v2))};
}

inline bool intersect(const AABB *a, const AABB *b)
{
  return (a->min.x <= b->max.x && a->max.x >= b->min.x) &&
         (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
         (a->min.z <= b->max.z && a->max.z >= b->min.z);
}

inline std::vector<Triangle> to_triangles(const std::vector<glm::vec4> primitives)
{
  std::vector<Triangle> triangles;

  for (uint i = 0; i < primitives.size() / 3; i++)
  {
    Triangle t;
    t.v[0] = primitives[i * 3 + 0];
    t.v[1] = primitives[i * 3 + 1];
    t.v[2] = primitives[i * 3 + 2];
    triangles.push_back(t);
  }

  return triangles;
}

template <class Bounded>
class KdTree
{
public:
  KdTree(const std::vector<Bounded> primitives)
  {
    uint id = construct(primitives, bounds(primitives), 0);
    assert(id == 0);
  }

  uint construct(const std::vector<Bounded> primitives, const AABB &bounds, int depth = 0)
  {
    if (primitives.size() < 8 || depth > 5)
    {
      uint offset = m_primitives.size();
      uint node_id = m_nodes.size();

      m_primitives.insert(m_primitives.end(), primitives.begin(), primitives.end());

      KdNode node;
      node.min = bounds.min;
      node.max = bounds.max;
      node.left = INVALID;
      node.right = INVALID;
      node.offset = offset;
      node.count = primitives.size();
      m_nodes.push_back(node);
      return node_id;
    }
    else
    {
      KdNode node;
      node.min = bounds.min;
      node.max = bounds.max;
      node.offset = INVALID;
      node.count = 0;

      uint node_id = m_nodes.size();
      m_nodes.push_back({});

      // simplest heuristic
      int axis = depth % 3;
      AABB left_aabb, right_aabb;

      left_aabb.min = bounds.min;
      right_aabb.max = bounds.max;

      float boundary = bounds.min[axis] + (bounds.max[axis] - bounds.min[axis]) / 2.0f;
      left_aabb.max[axis] = boundary;
      right_aabb.min[axis] = boundary;

      std::vector<Bounded> left, right;

      for (auto primtive : primitives)
      {
        AABB aabb = primtive.bounds();

        if (intersect(&left_aabb, &aabb))
        {
          left.push_back(primtive);
        }

        if (intersect(&right_aabb, &aabb))
        {
          right.push_back(primtive);
        }
      }

      node.left = construct(left, left_aabb, depth + 1);
      node.right = construct(right, right_aabb, depth + 1);

      m_nodes[node_id] = node;
      return node_id;
    }
  }

  static AABB bounds(const std::vector<Bounded> primitives)
  {
    AABB total;
    total.min = glm::vec4(+1e5f);
    total.max = glm::vec4(-1e5f);

    for (auto &primitive : primitives)
    {
      AABB aabb = primitive.bounds();
      total.min = glm::min(aabb.min, total.min);
      total.max = glm::max(aabb.max, total.max);
    }
    return total;
  }

  std::vector<KdNode> nodes() { return m_nodes; }
  std::vector<Bounded> primitives() { return m_primitives; }

private:
  std::vector<KdNode> m_nodes;
  std::vector<Bounded> m_primitives;
};
