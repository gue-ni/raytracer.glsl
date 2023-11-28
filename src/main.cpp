#include "renderer.h"

#define CORNELL_BOX 0

int main()
{
  Renderer renderer(800, 600);

  float r = 25000;
  float w = 16.0f;
  float h = 9.0f;
  float l = 100.0f;
  float sr = 3.0f;

  // setup spheres
  const std::vector<Sphere> spheres = {
    { {+7.0f, -h + sr, 7.5f}, sr, 0 },
    { {-7.0f, -h + sr, 6.5f}, sr, 6 },
    { {0.0f, -h + sr, 7.0f}, sr, 5 },
    { { 0.0f, -(r + h), 7.0f}, r, 3 },
#if CORNELL_BOX
    { { 0.0f, h + l * 0.999f , 7.0f}, l, 1 },
    { { 0.0f, +(r + h), 7.0f}, r, 3 },
    { { 0.0f, 0.0f, 7.0f + (r + w)}, r, 7 },
    { { 0.0f, 0.0f, -(r + w)}, r, 3 },
    { { -(r + w), 0.0f, 7.0f}, r, 4 },
    { { +(r + w), 0.0f, 7.0f}, r, 2 },
#else // outside
    { { 3.0f, h + 10.0f , 7.0f}, 3.0, 1 },
#endif
  };

  renderer.set_spheres(spheres);

  // setup material 
  const std::vector<Material> materials = {
    { {0.75f, 0.75f, 0.75f, 0.75f }, glm::vec4(0.0f), Material::MaterialType::DIFFUSE },
    { {0.75f, 0.75f, 0.75f, 0.0f}, glm::vec4(20.0f) },
    { {0.75f, 0.00f, 0.00f, 0.0f}, glm::vec4(0.0f) },
    { {0.99f, 0.99, 0.99, 0.0f}, glm::vec4(0.0f) },
    { {0.00f, 0.75f, 0.00f, 0.0f}, glm::vec4(0.0f) },
    { {0.75f, 0.75f, 0.75f, 1.0f }, glm::vec4(0.0f), Material::MaterialType::SPECULAR },
    { {0.75f, 0.75f, 0.75f, 0.0f }, glm::vec4(0.0f), Material::MaterialType::TRANSMISSIVE },
    { {0.00f, 0.00f, 0.75f, 0.0f}, glm::vec4(0.0f) },
  };

  renderer.set_materials(materials);

  // setup environment map
  const std::string cubemap_path = "assets/cubemap/";

  const std::array<std::string, 6>& faces = {
    cubemap_path + "right.png",  
    cubemap_path + "left.png",  
    cubemap_path + "top.png",
    cubemap_path + "bottom.png", 
    cubemap_path + "front.png", 
    cubemap_path + "back.png ",
  };

  auto envmap = std::make_unique<CubemapTexture>(faces);
  renderer.set_envmap(std::move(envmap));

  std::vector<glm::vec4> obj = Renderer::load_obj("assets/icosphere.obj");

  glm::mat4 m = Renderer::transform(glm::vec3(0.0f, -h + sr, 0.0f), glm::vec3(3.0f));

  for (glm::vec4& v : obj)
  {
    v = m * v;
  }

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

  renderer.run();
  return 0;
}
