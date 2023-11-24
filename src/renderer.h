#include "window.h"
#include "gfx/gfx.h"

#include <memory>

using namespace gfx::gl;

#if defined(__GNUC__) || defined(__clang__)
#  define ALIGN_START(x)
#  define ALIGN_END(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#  define ALIGN_START(x) __declspec(align(x))
#  define ALIGN_END(x)
#else
#  error "Unknown compiler; can't define ALIGN"
#endif

ALIGN_START(16) struct Sphere {
  glm::vec3 center; 
  float radius;
  int material = 0;
} ALIGN_END(16);

// vec4 only for alignment purposes
ALIGN_START(16) struct Material {

  enum MaterialType: unsigned int {
    DIFFUSE       = 0,
    SPECULAR      = 1,
    TRANSMISSIVE  = 2,
  };

  glm::vec4 albedo;
  glm::vec3 emission;
  MaterialType type = DIFFUSE;
} ALIGN_END(16);

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

  Camera(const glm::vec3& position_) 
    : position(position_)
  {}
};

class Renderer : public Window {
public:
  Renderer(int width, int height);
  void render(float dt) override;
  void event(const SDL_Event &event) override;
  void keyboard_state(const Uint8* state) override;

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
