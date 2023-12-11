#include "kdtree.h"

KdTree::KdTree(const std::vector<glm::vec4> vertices)
{
  // three vertices per triangle
  assert(vertices.size() % 3 == 0);

  uint id = construct(vertices, bounds(vertices), 0);
  assert(id == 0);
  
}


uint KdTree::construct(const std::vector<glm::vec4> vertices, const AABB& bounds, int depth)
{
  if (triangle_count(vertices) < 8 || depth > 5) {
    // leaf node

    uint offset = m_vertices.size();
    uint node_id = m_nodes.size();

    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());

    KdNode node; 
    node.min = bounds.min;
    node.max = bounds.max;
    node.left = INVALID;
    node.right = INVALID;
    node.offset = offset;
    node.count = vertices.count();
    m_nodes.push_back(node);
    return node_id;
  } else {

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

    std::vector<glm::vec4> left, right;

    for (uint i = 0; i < vertices.size() / 3; i++) {
      glm::vec4 v0 = vertices[i * 3  + 0];
      glm::vec4 v1 = vertices[i * 3  + 1];
      glm::vec4 v2 = vertices[i * 3  + 2];

      AABB aabb = from_vertices(v0, v1, v2);

      if (intersect(&left_aabb, &aabb)) {
        left.insert(left.end(), {v0, v1, v2});
      }

      if (intersect(&right_aabb, &aabb)) {
        right.insert(right.end(), {v0, v1, v2});
      }
    }

    node.left = construct(left, left_aabb, depth + 1);
    node.right = construct(right, right_aabb, depth + 1);

    m_nodes[node_id] = node;
    return node_id;
  }
}


AABB KdTree::bounds(const std::vector<glm::vec4> vertices) 
{
  AABB aabb;
  aabb.min = glm::vec4(+1e5f);
  aabb.max = glm::vec4(-1e5f);
  for (auto& vertex : vertices) {
    aabb.min = glm::min(aabb.min, vertex);
    aabb.max = glm::max(aabb.max, vertex);
  }
  return aabb;
}



