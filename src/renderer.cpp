#include "renderer.h"

Renderer::Renderer(int width, int height) 
  : Window(width, height)
  , m_screen_shader(std::make_unique<ShaderProgram>(ShaderProgram::string_from_file("shaders/screen.vert"), ShaderProgram::string_from_file("shaders/screen.frag")))
  , m_render_shader(std::make_unique<ShaderProgram>(ShaderProgram::string_from_file("shaders/pathtracer.comp")))
  , m_texture(std::make_unique<Texture>())
  , m_screen_quad_vao(std::make_unique<VertexArrayObject>())
  , m_screen_quad_vbo(std::make_unique<VertexBuffer>())
{

  // setup screen quad
  const glm::vec2 size = glm::vec2(0.25f);

  const std::vector<glm::vec2> vertices = {
      {-size.x, +size.y}, {0, 1},  // top left
      {-size.x, -size.y}, {0, 0},  // bottom left
      {+size.x, +size.y}, {1, 1},  // top right

      {+size.x, +size.x}, {1, 1},  // top right
      {-size.x, -size.y}, {0, 0},  // bottom left
      {+size.x, -size.y}, {1, 0},  // bottom right
  };

  m_screen_quad_vao->bind();
  m_screen_quad_vbo->bind();
  m_screen_quad_vbo->buffer(vertices);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)(sizeof(glm::vec2)));
  glEnableVertexAttribArray(1);

  m_screen_quad_vao->unbind();

  // setup texture
  m_texture->bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
}

void Renderer::render(float dt)
{
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

#if 0
  // dispatch compute shaders
  m_render_shader->bind();
  m_render_shader->set_uniform("u_frames", m_frames);
  m_render_shader->set_uniform("u_samples", 8);
  m_render_shader->set_uniform("u_max_depth", 5);
  glBindImageTexture(0, m_texture->id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
  glDispatchCompute(m_width, m_height, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
#endif

  m_texture->bind(0);

  // draw scren quad
  m_screen_shader->bind();
  m_screen_shader->set_uniform("u_texture", 0);
  m_screen_quad_vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, 3 * 2);
}

void Renderer::event(const SDL_Event &event)
{
  // TODO: handle user input
  // TODO: move camera
}