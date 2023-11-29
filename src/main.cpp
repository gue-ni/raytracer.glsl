#include "renderer.h"

#ifndef CORNELL_BOX
#define CORNELL_BOX 0
#endif

void setup_scene(Renderer& renderer)
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
    Sphere( {-18.0f, -room_size.y + sr, 0.0f}, sr, 7 ),
    Sphere( { -6.0f, -room_size.y + sr, 0.0f}, sr, 5 ),
    Sphere( {+18.0f, -room_size.y + sr, 0.0f}, sr, 4 ),
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

  std::vector<glm::vec4> obj = Renderer::load_obj("assets/cube.obj");

  //std::vector<glm::vec4> all;

  glm::mat4 matrix = Renderer::transform(glm::vec3(-6.0f, -room_size.y + sr, 0.0f), glm::vec3(sr), glm::quat(glm::vec3(0.0f, M_PI / 4, 0.0f)));
  for (glm::vec4& vertex : obj) vertex = matrix * vertex;

  renderer.set_vertices(obj);

  const std::vector<Mesh> meshes = {
    Mesh(0, obj.size() / 3, 0),
  };

  renderer.set_meshes(meshes);
#else
  std::vector<glm::vec4> obj = Renderer::load_obj("assets/icosphere.obj");

  glm::mat4 matrix = Renderer::transform(glm::vec3(6.0f, -room_size.y + sr, 0.0f), glm::vec3(sr));
  for (glm::vec4& vertex : obj) vertex = matrix * vertex;

  renderer.set_vertices(obj);

  const std::vector<Mesh> meshes = {
    Mesh(0, obj.size() / 3, 6),
  };

  renderer.set_meshes(meshes);
#endif
}

int main()
{
#define RES 2
#if    (RES == 0)
  glm::ivec2 full_hd = glm::ivec2(1920, 1080);
#elif (RES == 1)
  glm::ivec2 standard_hd = glm::ivec2(1280, 720);
#elif (RES == 2)
  glm::ivec2 res = glm::ivec2(768, 480);
#else
#error unknown resolution
#endif

  Renderer renderer(res.x, res.y);
  setup_scene(renderer);
  renderer.run();
  return 0;
}
