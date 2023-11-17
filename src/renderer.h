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

ALIGN(16) struct Material {
  glm::vec3 albedo;
  glm::vec3 emission;
};

struct Camera {
  glm::vec3 position;
  glm::vec3 target;
  float fov;
  float aspect_ratio;
  float focal_length;
  float aperture;

  Camera(const glm::vec3& position_, const glm::vec3& target_) 
    : position(position_), target(target_), fov(45.0f)
  {}
};

class Renderer : public Window {
public:
  Renderer(int width, int height);
  void render(float dt) override;
  void event(const SDL_Event &event) override;
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

  float m_timer = 0;
};
