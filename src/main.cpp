#include "renderer.h"

#include <random>

#include <json.hpp>

#ifndef CORNELL_BOX
#define CORNELL_BOX 0
#endif

float random(float min = 0, float max = 1)
{
  static std::default_random_engine e;
  static std::uniform_real_distribution<> dis(min, max); // range [min, max)
  return dis(e);
}

glm::vec3 random(const glm::vec3& min, const glm::vec3& max) 
{
  return {
    random(min.x, max.x),
    random(min.y, max.y),
    random(min.z, max.z)
  };
}

void setup_scene_01(Renderer& renderer)
{
  float r = 10000;
  float l = 100.0f;
  float sr = 4.0f;

  glm::vec3 room_size = glm::vec3(16.0f, 16.0f, 16.0f);

  // setup spheres
  const std::vector<Sphere> spheres = {
    Sphere( { 0.0f, -(room_size.y + r), 0.0f}, r, 0 ), // ground
#if CORNELL_BOX
    Sphere( { 0.0f, room_size.y + l * 0.998f , 0.0f}, l, 1 ),
    Sphere( { 0.0f, +(r + room_size.y), 0.0f}, r, 0 ),
    Sphere( { 0.0f, 0.0f, +(r + room_size.z)}, r, 0 ),
    Sphere( { -(r + room_size.x), 0.0f, 0.0f}, r, 3 ),
    Sphere( { +(r + room_size.x), 0.0f, 0.0f}, r, 2 ),
    Sphere( { +7.0f, -room_size.y + sr + 2, 3.0f}, sr + 2, 5 ),
#else 
    Sphere( { 3.0f, room_size.y + 10.0f , 0.0f}, 3.0, 1 ),
    Sphere( {-18.0f, -room_size.y + sr, 0.0f}, sr, 4 ),
    Sphere( { -6.0f, -room_size.y + sr, 0.0f}, sr, 5 ),
    Sphere( {+18.0f, -room_size.y + sr, 0.0f}, sr, 7 ),
#endif
  };

  renderer.set_spheres(spheres);

  // setup material 
  const std::vector<Material> materials = {
    Material(gfx::rgb(0xAAAAAA)),
    Material(gfx::rgb(0xFFFFFF), gfx::rgb(0xFFFEFA) * 30.0f),
    Material(gfx::rgb(0xBC0000)), // red wall
    Material(gfx::rgb(0x00BC00)), // green wall
    Material(gfx::rgb(0xAAAAAA), gfx::rgb(0x0), 1.0f, MaterialType::SPECULAR ),
    Material(gfx::rgb(0xFFFFFF), gfx::rgb(0x0), 0.0f, MaterialType::TRANSMISSIVE ),
    Material(gfx::rgb(0xFF5733), gfx::rgb(0x0), 0.0f, MaterialType::TRANSMISSIVE), // ruby
    Material(gfx::rgb(0xAAAAAA), gfx::rgb(0x0), 0.5f, MaterialType::SPECULAR ),
  };

  renderer.set_materials(materials);

#if (!CORNELL_BOX)
  // setup environment map
  const std::array<std::string, 6>& faces = {
    "assets/cubemap/right.png",  
    "assets/cubemap/left.png",  
    "assets/cubemap/top.png",
    "assets/cubemap/bottom.png", 
    "assets/cubemap/front.png", 
    "assets/cubemap/back.png ",
  };

  auto envmap = std::make_unique<CubemapTexture>(faces);
  renderer.set_envmap(std::move(envmap));
#endif

#if (CORNELL_BOX)

  std::vector<glm::vec4> obj = Renderer::load_obj("assets/models/cube.obj");

  //std::vector<glm::vec4> all;

  glm::mat4 matrix = Renderer::transform(glm::vec3(-6.0f, -room_size.y + sr, 0.0f), glm::vec3(sr), glm::quat(glm::vec3(0.0f, M_PI / 4, 0.0f)));
  for (glm::vec4& vertex : obj) vertex = matrix * vertex;

  renderer.set_vertices(obj);

  const std::vector<Mesh> meshes = {
    Mesh(0, obj.size() / 3, 0),
  };

  renderer.set_meshes(meshes);
#else
  std::vector<glm::vec4> obj = Renderer::load_obj("assets/models/icosphere.obj");

  glm::mat4 matrix = Renderer::transform(glm::vec3(6.0f, -room_size.y + sr, 0.0f), glm::vec3(sr));
  for (glm::vec4& vertex : obj) {
    vertex = matrix * vertex;
    vertex.w = 0; // mesh material
  }

  renderer.set_vertices(obj);

  renderer.set_kdtree(obj);


  const std::vector<Mesh> meshes = {
    Mesh(0, obj.size() / 3, 6),
  };

  renderer.set_meshes(meshes);
#endif
}

void setup_scene_02(Renderer& renderer)
{
  const std::vector<Material> materials = {
    /* 0 */ Material(gfx::rgb(0xff0000)),
    /* 1 */ Material(gfx::rgb(0xff00ff)),
  };

  renderer.set_materials(materials);

  float r = 2;

#if 0
  const std::vector<Sphere> spheres = {
    Sphere( {  0.0f, 0.0f, 0.0f}, r, 0),
    Sphere( { 10.0f, 0.0f, 0.0f}, r, 0),
    Sphere( { 20.0f, 0.0f, 0.0f}, r, 0),
  };
#else
  // random spheres
  auto min = glm::vec3(-20.0f);
  auto max = glm::vec3(+20.0f);
  uint material = 0;
  std::vector<Sphere> spheres;

  for (int i = 0; i < 100; i++) {
    spheres.push_back(Sphere( random(min, max), r, material));
  }
#endif

  KdTree<Sphere, 8, 1> tree(spheres);

  std::vector<glm::vec4> mesh = Renderer::load_obj("assets/models/icosphere.obj");
  glm::mat4 matrix = Renderer::transform(glm::vec3(30.0f, 0.0f, 0.0f), glm::vec3(1.0f));
  for (glm::vec4& vertex : mesh) {
    vertex = matrix * vertex;
    vertex.w = static_cast<float>(0); // encode material
  }

  //auto triangles = to_triangles(mesh);
  //KdTree<Triangle> triangle_tree(triangles);

  auto nodes = tree.nodes();
  auto primitives = tree.primitives();


  //for (auto& p : primitives) std::cout << p << std::endl;
  for (auto& n : nodes) std::cout << n << std::endl;
  
  printf("original: %zd, primitives: %zd\n", spheres.size(), primitives.size());

  renderer.set_nodes(nodes);
  renderer.set_spheres(primitives);

  const std::array<std::string, 6>& faces = {
    "assets/cubemap/right.png",  
    "assets/cubemap/left.png",  
    "assets/cubemap/top.png",
    "assets/cubemap/bottom.png", 
    "assets/cubemap/front.png", 
    "assets/cubemap/back.png ",
  };

  auto envmap = std::make_unique<CubemapTexture>(faces);
  renderer.set_envmap(std::move(envmap));
}



void setup_scene_03(Renderer& renderer)
{

}

int main()
{
  Renderer renderer(1080, 720);
  setup_scene_02(renderer);
  renderer.run();
  return 0;
}
