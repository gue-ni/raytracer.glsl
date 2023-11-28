#include "renderer.h"

#define CORNELL_BOX 0

void setup_scene(Renderer& renderer)
{
  float r = 25000;
  float l = 100.0f;
  float sr = 5.0f;

  glm::vec3 room_size = glm::vec3(16.0f, 16.0f, 16.0f);

  // setup spheres
  const std::vector<Sphere> spheres = {

    { { 0.0f, -(room_size.y + r), 0.0f}, r, 3 }, // ground

#if CORNELL_BOX
    { { 0.0f, room_size.y + l * 0.999f , 0.0f}, l, 1 },
    { { 0.0f, +(r + room_size.y), 0.0f}, r, 3 },
    { { 0.0f, 0.0f, +(r + room_size.z)}, r, 3 },
    //{ { 0.0f, 0.0f, -(r + room_size.z * 5.0f)}, r, 3 },
    { { -(r + room_size.x), 0.0f, 0.0f}, r, 4 },
    { { +(r + room_size.x), 0.0f, 0.0f}, r, 2 },
#else 
    { { 3.0f, room_size.y + 10.0f , 0.0f}, 3.0, 1 },

    { {-18.0f, -room_size.y + sr, 0.0f}, sr, 0 },
    { { -6.0f, -room_size.y + sr, 0.0f}, sr, 6 },
    { {+18.0f, -room_size.y + sr, 0.0f}, sr, 5 },

#endif
  };

  renderer.set_spheres(spheres);

  // setup material 
  const std::vector<Material> materials = {
    { {0.75f, 0.75f, 0.75f, 0.75f }, glm::vec4(0.0f), Material::MaterialType::DIFFUSE },
    { {0.75f, 0.75f, 0.75f, 0.0f}, glm::vec4(25.0f) },
    { {0.75f, 0.00f, 0.00f, 0.0f}, glm::vec4(0.0f) },
    { {0.99f, 0.99, 0.99, 0.0f}, glm::vec4(0.0f) },
    { {0.00f, 0.75f, 0.00f, 0.0f}, glm::vec4(0.0f) },
    { {0.75f, 0.75f, 0.75f, 1.0f }, glm::vec4(0.0f), Material::MaterialType::SPECULAR },
    { {0.75f, 0.75f, 0.75f, 0.0f }, glm::vec4(0.0f), Material::MaterialType::TRANSMISSIVE },
    { {0.00f, 0.00f, 0.75f, 0.0f}, glm::vec4(0.0f) },
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

  glm::mat4 matrix = Renderer::transform(glm::vec3(-4.0f, -room_size.y + sr, 0.0f), glm::vec3(sr), glm::quat(glm::vec3(0.0f, 1.2f, 0.0f)));
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

  const std::vector<glm::vec4> vertices = {
    glm::vec4(-2, 0, 0, 0),
    glm::vec4(-2, 2, 0, 0),
    glm::vec4(+2, 0, 0, 0),

    glm::vec4(-2, 2, 0, 0),
    glm::vec4(+2, 2, 0, 0),
    glm::vec4(+2, 0, 0, 0),
  };

  const std::vector<Mesh> meshes = {
    Mesh(0, obj.size() / 3, 6),
  };

  renderer.set_meshes(meshes);
#endif
}

int main()
{
  Renderer renderer(800, 800);
  setup_scene(renderer);
  renderer.run();
  return 0;
}
