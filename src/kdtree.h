#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

#include <vector>
#include <limits>
#include <ostream>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <stack>

using uint = unsigned int;

constexpr uint INVALID = std::numeric_limits<uint>::max();


struct Ray 
{
  glm::vec3 origin;
  glm::vec3 direction;
};

struct AABB
{
  glm::vec4 min;
  glm::vec4 max;
};

inline bool intersect(const AABB *a, const AABB *b)
{
  return (a->min.x <= b->max.x && a->max.x >= b->min.x) &&
         (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
         (a->min.z <= b->max.z && a->max.z >= b->min.z);
}

// https://tavianator.com/2011/ray_box.html
// https://tavianator.com/2022/ray_box_boundary.html
inline bool intersect(const Ray *ray, const AABB *box)
{
  float tmin = 0.0, tmax = 1e6;

  for (int d = 0; d < 3; ++d) {
    float t1 = (box->min[d] - ray->origin[d]) / ray->direction[d];
    float t2 = (box->max[d] - ray->origin[d]) / ray->direction[d];

    tmin = glm::max(tmin, glm::min(t1, t2));
    tmax = glm::min(tmax, glm::max(t1, t2));
  }

  return tmin < tmax;
}

inline std::ostream &operator<<(std::ostream &os, const AABB &obj)
{
  os << "AABB { min = " << obj.min << ", max = " << obj.max << " }";
  return os;
}


struct KdNode : public AABB
{
  uint left = INVALID;
  uint right = INVALID;
  uint offset = 0;
  uint count = 0;
};

template <class Bounded, uint NODE_SIZE = 8, uint MAX_DEPTH = 5>
class KdTree
{
public:
  KdTree(const std::vector<Bounded> primitives)
  {
    (void)construct(primitives, bounds(primitives), 0);
  }

  uint construct(std::vector<Bounded> primitives, const AABB &bounds, uint depth = 0)
  {
    if (primitives.size() <= NODE_SIZE || depth >= MAX_DEPTH)
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
      printf("Node = %u, offset = %u, count = %u\n", node_id, node.offset, node.count);
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

      int axis = depth % 3;
      AABB left_aabb, right_aabb;

      left_aabb.min = bounds.min;
      right_aabb.max = bounds.max;

#if 0
      // simplest heuristic
      float boundary = bounds.min[axis] + (bounds.max[axis] - bounds.min[axis]) / 2.0f;
#else
      auto compare = [&axis](const Bounded &a, const Bounded &b) { 
        return a.bounds().min[axis] < b.bounds().min[axis]; 
      };

      std::sort(primitives.begin(), primitives.end(), compare);
      float boundary = primitives[primitives.size() / 2].bounds().min[axis];
#endif
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
      printf("Node = %u, left = %u, right = %u\n", node_id, node.left, node.right);
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

  // non-recursive kd-tree traversal
  std::vector<Bounded> traverse(const Ray& ray) const {

    uint id = 0; 

    std::stack<uint> stack;
    stack.push(id);

    std::vector<Bounded> result;

    while (!stack.empty()) 
    {
      id = stack.pop();

      KdNode node = m_nodes[id];

      if (!intersect(&ray, &node))
        continue;

      if (node.left != INVALID)  
        stack.push(node.left);
      
      if (node.right != INVALID) 
        stack.push(node.right);

      if (0 < node.count) {
        result.insert(result.end(), m_primitives.begin() + node.offset, m_primitives.begin() + node.offset + node.count);
      }
    }

    return result;
  }

private:
  std::vector<KdNode> m_nodes;
  std::vector<Bounded> m_primitives;
};
