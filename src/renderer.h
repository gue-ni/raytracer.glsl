#include "window.h"
#include "gfx/gfx.h"
#include "kdtree.h"

#include <memory>
#include <vector>

using namespace gfx::gl;

using uint = unsigned int;

#if defined(__GNUC__) || defined(__clang__)
#  define ALIGN_START(x)
#  define ALIGN_END(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#  define ALIGN_START(x) __declspec(align(x))
#  define ALIGN_END(x)
#else
#  error "Unknown compiler; can't define ALIGN"
#endif

ALIGN_START(16) 
struct Sphere {
  glm::vec3 center; 
  float radius;
  int material = 0;
  Sphere(const glm::vec3& center_, float radius_, int mat = 0)
    : center(center_), radius(radius_), material(mat) {}

  AABB bounds() const {
    return { glm::vec4(center - radius, 1.0f), glm::vec4(center + radius, 1.0f) };
  }
  
} ALIGN_END(16);

enum MaterialType: uint {
  DIFFUSE       = 0,
  SPECULAR      = 1,
  TRANSMISSIVE  = 2,
};

// vec4 only for alignment purposes
ALIGN_START(16) 
struct Material {
  glm::vec4 albedo;
  glm::vec3 emission;
  MaterialType type;

  Material(const glm::vec3& albedo_, const glm::vec3& emission_ = glm::vec3(0.0f), 
    float smoothness = 0.0f, const MaterialType& type_ = DIFFUSE) 
    : albedo(albedo_, smoothness), emission(emission_), type(type_) {}
} ALIGN_END(16);


ALIGN_START(16) struct Mesh {
  uint start; // start offset
  uint size; // triangle count
  int material;

  Mesh(uint start_, uint size_, int mat = 0) 
    : start(start_), size(size_), material(mat) {}
};

inline glm::vec3 vector_from_spherical(float pitch, float yaw)
{
    return {
        std::cos(yaw) * std::sin(pitch),
        std::cos(pitch),
        std::sin(yaw) * std::sin(pitch)
     };
}

struct Camera {
  glm::vec3 position;

  float fov = 45.0f;
  
  float focal_length  = 10.0f;
  float aperture      = 0.001f;
  
  float pitch = M_PI / 2;
  float yaw   = M_PI / 2;

  glm::vec3 forward = {0.0f, 0.0f, 1.0f};
  glm::vec3 up      = {0.0f, 1.0f, 0.0f};
  glm::vec3 right   = {-1.0f, 0.0f, 0.0f};

  Camera(const glm::vec3& position_, float fov_) 
    : position(position_), fov(fov_)
  {}
};

class Renderer : public Window {
public:
  Renderer(int width, int height);
  void render(float dt) override;
  void event(const SDL_Event &event) override;
  void keyboard_state(const Uint8* state) override;

  void set_spheres(const std::vector<Sphere>& spheres);
  void set_materials(const std::vector<Material>& material);
  void set_envmap(std::unique_ptr<CubemapTexture> envmap);
  void set_vertices(const std::vector<glm::vec4>& vertices);
  void set_meshes(const std::vector<Mesh>& meshes);
  void set_kdtree(const std::vector<glm::vec4>& vertices);

  static std::vector<glm::vec4> load_obj(const std::string& path);
  static glm::mat4 transform(const glm::vec3& translate, const glm::vec3& scale, const glm::quat& rotate = glm::quat(glm::vec3(0.0f)));


private:
  std::unique_ptr<ShaderProgram> m_screen_shader = nullptr; 
  std::unique_ptr<ShaderProgram> m_render_shader = nullptr;

  std::unique_ptr<VertexArrayObject> m_screen_quad_vao = nullptr;
  std::unique_ptr<VertexBuffer> m_screen_quad_vbo = nullptr;
  
  std::unique_ptr<Texture> m_texture = nullptr;

  std::unique_ptr<CubemapTexture> m_envmap = nullptr;

  std::unique_ptr<ShaderStorageBuffer> m_spheres = nullptr;
  std::unique_ptr<ShaderStorageBuffer> m_materials = nullptr;
  std::unique_ptr<ShaderStorageBuffer> m_vertices = nullptr;
  std::unique_ptr<ShaderStorageBuffer> m_meshes = nullptr;
  std::unique_ptr<ShaderStorageBuffer> m_kdtree = nullptr;

  int m_bounces = 5;
  unsigned int m_samples = 1;

  Camera m_camera;

  bool m_reset = false;
  bool m_mousedown = false;
  bool m_use_envmap = true;
  bool m_use_dof = true;

  void reset_buffer();
  void save_to_file() const;


#if 0
  glm::vec3 m_background = glm::vec3(1.0f);
#else
  glm::vec3 m_background = glm::vec3(0.52f, 0.80f, 0.92f);
#endif

  float m_timer = 0;
};
