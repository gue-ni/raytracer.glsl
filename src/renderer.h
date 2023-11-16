#include "window.h"
#include "gfx/gfx.h"

#include <memory>

using namespace gfx::gl;

struct Sphere {
  glm::vec3 center; 
  float radius;
};

class Renderer : public Window {
public:
  Renderer(int width, int height);
  void render(float dt) override;

private:
  std::unique_ptr<ShaderProgram> m_screen_shader = nullptr; 
  std::unique_ptr<ShaderProgram> m_render_shader = nullptr;

  std::unique_ptr<VertexArrayObject> m_screen_quad_vao = nullptr;
  std::unique_ptr<VertexBuffer> m_screen_quad_vbo = nullptr;
  
 std::unique_ptr<Texture> m_texture = nullptr;
};