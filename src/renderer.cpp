#include "renderer.h"

Renderer::Renderer(int width, int height) 
  : Window(width, height)
  , m_screen_shader(std::make_unique<Shader>(Shader::string_from_file("shaders/screen.vert"), Shader::string_from_file("shaders/screen.frag")))
  , m_render_shader(std::make_unique<Shader>(Shader::string_from_file("shaders/pathtracer.comp")))
  , m_texture(std::make_unique<Texture>())
  , m_screen_quad_vao(std::make_unique<VertexArrayObject>())
  , m_screen_quad_vbo(std::make_unique<VertexBuffer>())
{
}

void Renderer::render(float dt)
{
  // dispatch compute shaders
  // draw scren quad
}