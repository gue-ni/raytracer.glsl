#include "renderer.h"

Renderer::Renderer(int width, int height) 
  : Window(width, height)
  , m_screen_shader(std::make_unique<ShaderProgram>(ShaderProgram::string_from_file("shaders/screen.vert"), ShaderProgram::string_from_file("shaders/screen.frag")))
  , m_render_shader(std::make_unique<ShaderProgram>(ShaderProgram::string_from_file("shaders/pathtracer.comp")))
  , m_texture(std::make_unique<Texture>())
  , m_screen_quad_vao(std::make_unique<VertexArrayObject>())
  , m_screen_quad_vbo(std::make_unique<VertexBuffer>())
  , m_spheres(std::make_unique<ShaderStorageBuffer>())
  , m_materials(std::make_unique<ShaderStorageBuffer>())
  , m_camera(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
{

  //static_assert(sizeof(Material) == sizeof(float) * 6);
  //static_assert(sizeof(Sphere) == (sizeof(float) * 4 + sizeof(int)));
  //static_assert(sizeof(Sphere) == (sizeof(float) * 7));

  // setup screen quad
  const glm::vec2 size = glm::vec2(1.0f);

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
  m_screen_quad_vbo->buffer_data(std::span(vertices));

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (void *)(sizeof(glm::vec2)));
  glEnableVertexAttribArray(1);

  m_screen_quad_vao->unbind();

  // setup texture
  m_texture->bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);

  // setup spheres
  std::vector<Sphere> spheres = {
    { {+2.5f, 0.0f, 7.0f}, 1.0f, 0 },
    { { 0.0f, 0.0f, 7.0f}, 1.0f, 1 },
    { {-2.5f, 0.0f, 7.0f}, 1.0f, 0 },
    //{ {0.0f, -1000.0f, 0.0f}, 1000.0f, 0 },
  };

  m_spheres->bind();
  m_spheres->buffer_data(std::span(spheres));

#if 1
  // setup material 
  std::vector<Material> materials = {
    { {0.75f, 0.0f, 0.0f }, glm::vec3(0.0f) },
    { {0.75f, 0.75f, 0.75f}, glm::vec3(1.0f) },
  };

  m_materials->bind();
  m_materials->buffer_data(std::span(materials));
#endif
}

void Renderer::render(float dt)
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  m_spheres->bind_buffer_base(1);
  m_materials->bind_buffer_base(2);

  m_render_shader->bind();
  m_render_shader->set_uniform("u_frames", m_frames);
  m_render_shader->set_uniform("u_samples", 8);
  m_render_shader->set_uniform("u_max_bounce", 1);
  m_render_shader->set_uniform("u_camera_position", m_camera.position);
  m_render_shader->set_uniform("u_camera_target", m_camera.target);

  glBindImageTexture(0, m_texture->id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

  // dispatch compute shaders
  glDispatchCompute(m_width, m_height, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  m_texture->bind(0);

  // draw scren quad
  m_screen_shader->bind();
  m_screen_shader->set_uniform("u_texture", 0);
  m_screen_quad_vao->bind();
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::event(const SDL_Event &event)
{
  m_quit = (event.key.keysym.sym == SDLK_ESCAPE);
  // TODO: handle user input
  // TODO: move camera
}