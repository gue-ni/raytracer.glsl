#include "window.h"
#include "gfx/gfx.h"

#include <memory>

class Renderer : public Window {
public:
  Renderer(int width, int height);
  void render(float dt) override;

private:
  std::unique_ptr<gfx::gl::Shader> m_screen_shader = nullptr; 
  std::unique_ptr<gfx::gl::Shader> m_render_shader = nullptr;

  std::unique_ptr<gfx::gl::VertexArrayObject> m_screen_quad_vao = nullptr;
  std::unique_ptr<gfx::gl::VertexBuffer> m_screen_quad_vbo = nullptr;
  
  std::unique_ptr<gfx::gl::Texture> m_texture = nullptr;
};