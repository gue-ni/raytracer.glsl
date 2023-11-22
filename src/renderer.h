#include "window.h"
#include "gfx/gfx.h"

#include <memory>

using namespace gfx::gl;

#if defined(__GNUC__) || defined(__clang__)
#  define ALIGN(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#  define ALIGN(x) __declspec(align(x))
#else
#  error "Unknown compiler; can't define ALIGN"
#endif

ALIGN(16) struct Sphere {
  glm::vec3 center; 
  float radius;
  int material = 0;
};

// vec4 only for alignment purposes
ALIGN(16) struct Material {

  enum MaterialType: int {
    DIFFUSE       = 0,
    SPECULAR      = 1,
    TRANSMISSIVE  = 2,
  };

  glm::vec4 albedo;
  glm::vec3 emission;
  MaterialType type = DIFFUSE;
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
  float fov;
  float aspect_ratio;
  float focal_length;
  float aperture;
  
  float pitch = M_PI / 2;
  float yaw = M_PI / 2;
  float radius = 1.0f;

  glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
  glm::vec3 up      = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 right   = glm::vec3(-1.0f, 0.0f, 0.0f);

  Camera(const glm::vec3& position_) 
    : position(position_), fov(45.0f)
  {}
};

class Renderer : public Window {
public:
  Renderer(int width, int height);
  void render(float dt) override;
  void event(const SDL_Event &event) override;
  void keyboard_state(const Uint8* state) override;
  void save_to_file() const;

private:
  std::unique_ptr<ShaderProgram> m_screen_shader = nullptr; 
  std::unique_ptr<ShaderProgram> m_render_shader = nullptr;

  std::unique_ptr<VertexArrayObject> m_screen_quad_vao = nullptr;
  std::unique_ptr<VertexBuffer> m_screen_quad_vbo = nullptr;
  
  std::unique_ptr<Texture> m_texture = nullptr;

  std::unique_ptr<ShaderStorageBuffer> m_spheres = nullptr;
  std::unique_ptr<ShaderStorageBuffer> m_materials = nullptr;

  Camera m_camera;

  bool m_reset = false;
  bool m_mousedown = false;

#if 0
  glm::vec3 m_background = glm::vec3(1.0f);
#else
  glm::vec3 m_background = glm::vec3(0.52f, 0.80f, 0.92f);
#endif

  float m_timer = 0;
};
