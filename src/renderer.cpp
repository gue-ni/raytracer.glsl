#include "renderer.h"

Renderer::Renderer(int width, int height) 
  : Window(width, height)
  , m_screen_shader(std::make_unique<Shader>(Shader::string_from_file("shaders/screen.vert"), Shader::string_from_file("shaders/screen.frag")))
  , m_render_shader(std::make_unique<Shader>(Shader::string_from_file("shaders/pathtracer.comp")))
  , m_texture(std::make_unique<Texture>())
  , m_screen_quad_vao(std::make_unique<VertexArrayObject>())
  , m_screen_quad_vbo(std::make_unique<VertexBuffer>())
{

  constexpr glm::vec3 size = glm::vec3(0.25f);

  const std::vector<glm::vec2> vertices = {
      {-size.x, size.y},  {0, 1},  // top left
      {-size.x, -size.y}, {0, 0},  // bottom left
      {size.x, size.y},   {1, 1},  // top right

      {size.x, size.x},   {1, 1},  // top right
      {-size.x, -size.y}, {0, 0},  // bottom left
      {size.x, -size.y},  {1, 0},  // bottom right
  };

  m_screen_quad_vao->bind();
  m_screen_quad_vbo->bind();
  m_screen_quad_vbo->buffer(vertices);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)(sizeof(glm::vec2)));
  glEnableVertexAttribArray(1);

  m_screen_quad_vao->unbind();

}

void Renderer::render(float dt)
{
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // dispatch compute shaders
  // TODO

  // draw scren quad
  m_screen_shader->bind();
  m_screen_quad_vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, 3 * 2);
}