#include "renderer.h"
#include "gfx/gfx.h"

#include <span>
#include <iostream>
#include <chrono>

using namespace gfx;
using namespace gfx::gl;

Renderer::Renderer(int width, int height) 
  : Window(width, height)
  , m_screen_shader(std::make_unique<ShaderProgram>(ShaderProgram::string_from_file("shaders/screen.vert"), ShaderProgram::string_from_file("shaders/screen.frag")))
  , m_render_shader(std::make_unique<ShaderProgram>(ShaderProgram::string_from_file("shaders/pathtracer.glsl")))
  , m_texture(std::make_unique<Texture>())
  , m_screen_quad_vao(std::make_unique<VertexArrayObject>())
  , m_screen_quad_vbo(std::make_unique<VertexBuffer>())
  , m_spheres(std::make_unique<ShaderStorageBuffer>())
  , m_materials(std::make_unique<ShaderStorageBuffer>())
  , m_camera(glm::vec3(0.0f, 0.0f, -2.0f))
{
  // setup screen quad
  const std::vector<glm::vec2> vertices = {
      {-1, +1}, {0, 1},  // top left
      {-1, -1}, {0, 0},  // bottom left
      {+1, +1}, {1, 1},  // top right
      {+1, -1}, {1, 0},  // bottom right
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

  float r = 10000;
  float w = 5.0f;
  float h = 3.0f;
  float l = 10.0f;
  float sr = 1.1f;

  // setup spheres
  std::vector<Sphere> spheres = {
    { { 0.0f, h + l * 0.99f , 7.0f}, l, 1 },
    { {+2.5f, -h + sr, 7.5f}, sr, 0 },
    { {-2.5f, -h + sr, 6.5f}, sr, 6 },
    { {0.0f, -h + sr, 7.0f}, sr, 5 },

    { { 0.0f, -(r + h), 7.0f}, r, 3 },
    { { 0.0f, +(r + h), 7.0f}, r, 3 },
    { { 0.0f, 0.0f, 7.0f + (r + w)}, r, 7 },
    { { 0.0f, 0.0f, -(r + w)}, r, 3 },
    { { -(r + w), 0.0f, 7.0f}, r, 4 },
    { { +(r + w), 0.0f, 7.0f}, r, 2 },
  };

  m_spheres->bind();
  m_spheres->buffer_data(std::span(spheres));

  // setup material 
  std::vector<Material> materials = {
    { {0.75f, 0.75f, 0.75f, 0.75f }, glm::vec4(0.0f), Material::MaterialType::DIFFUSE },
    { {0.75f, 0.75f, 0.75f, 0.0f}, glm::vec4(20.0f) },
    { {0.75f, 0.00f, 0.00f, 0.0f}, glm::vec4(0.0f) },
    { {0.99f, 0.99, 0.99, 0.0f}, glm::vec4(0.0f) },
    { {0.00f, 0.75f, 0.00f, 0.0f}, glm::vec4(0.0f) },
    { {0.75f, 0.75f, 0.75f, 1.0f }, glm::vec4(0.0f), Material::MaterialType::SPECULAR },
    { {0.75f, 0.75f, 0.75f, 0.0f }, glm::vec4(0.0f), Material::MaterialType::TRANSMISSIVE },
    { {0.00f, 0.00f, 0.75f, 0.0f}, glm::vec4(0.0f) },
  };

  m_materials->bind();
  m_materials->buffer_data(std::span(materials));
}

void Renderer::render(float dt)
{
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  m_spheres->bind_buffer_base(1);
  m_materials->bind_buffer_base(2);

  m_render_shader->bind();
  m_render_shader->set_uniform("u_time", m_time);
  m_render_shader->set_uniform("u_frames", m_frames);
  m_render_shader->set_uniform("u_samples", m_samples);
  m_render_shader->set_uniform("u_max_bounce", m_bounces);
  m_render_shader->set_uniform("u_background", m_background);

  m_render_shader->set_uniform("u_camera_position", m_camera.position);
  m_render_shader->set_uniform("u_camera_fov", m_camera.fov);
  m_render_shader->set_uniform("u_camera_forward", m_camera.forward);
  m_render_shader->set_uniform("u_camera_right", m_camera.right);
  m_render_shader->set_uniform("u_camera_up", m_camera.up);
  
  m_render_shader->set_uniform("u_reset_flag", m_reset);
  if (m_reset) {
    m_reset = false;
    m_time = m_frames = 0;
  } 

  glBindImageTexture(0, m_texture->id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

  // dispatch compute shaders
  int work_group_size = 8;
  glDispatchCompute(m_width / work_group_size, m_height / work_group_size, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  m_texture->bind(0);

  // draw scren quad
  m_screen_shader->bind();
  m_screen_shader->set_uniform("u_texture", 0);
  m_screen_quad_vao->bind();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#if 1
  m_timer += dt;
  if (m_timer > 1)
  {
    m_timer = 0;
  }
#endif
}

void Renderer::save_to_file() const
{
  GLubyte* pixels = new GLubyte[m_width * m_height * 4]; 
  glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  auto now = std::chrono::system_clock::now();
  std::time_t timestamp = std::chrono::system_clock::to_time_t(now);

  Image image(pixels, m_width, m_height, 4);

  std::string filename = "render_" 
    + std::to_string(m_width) 
    + "x"
    + std::to_string(m_height)
    + "_"
    + std::to_string(timestamp)
    + "_"
    + std::to_string(m_frames)
    + ".png";

  if (image.write_png(filename, true)) {
    std::cout << "Wrote render to " << filename << std::endl;
  } else {
    std::cerr << "Could not write render to " << filename << std::endl;
  }
}

void Renderer::event(const SDL_Event &event)
{
  m_quit = (event.key.keysym.sym == SDLK_ESCAPE);

  switch (event.type)
  {
  case SDL_MOUSEBUTTONDOWN:
  {
    if (event.button.button == SDL_BUTTON_LEFT) {
      m_mousedown = true;
    } 
    break;
  }

  case SDL_MOUSEBUTTONUP:
  {
    if (event.button.button == SDL_BUTTON_LEFT) {
      m_mousedown = false;
    }
    break;
  }

  case SDL_MOUSEMOTION:
  {
    const float sensitivity = 0.01f;
    float delta_yaw = static_cast<float>(event.motion.xrel) * sensitivity;
    float delta_pitch = static_cast<float>(event.motion.yrel) * sensitivity;

    if (m_mousedown) {
      m_camera.yaw += delta_yaw;
      m_camera.pitch += delta_pitch;

      m_camera.forward = vector_from_spherical(m_camera.pitch, m_camera.yaw);
      m_camera.right = glm::normalize(glm::cross(m_camera.forward, glm::vec3(0.0f, 1.0f, 0.0f)));
      m_camera.up = glm::normalize(glm::cross(m_camera.right, m_camera.forward));

      m_reset = true;
    }
    break;
  }

  case SDL_MOUSEWHEEL:
  {
    int y = event.wheel.y; 
#if 1
    //m_camera.position.z += y;
#else
    m_camera.radius += glm::sign(y) * 0.5f;
#endif
    m_reset = true;
    break;
  }

  case SDL_KEYDOWN:
  {
    SDL_KeyboardEvent keyevent = event.key;
    if (keyevent.repeat != 0)
    {
      return;
    }

    switch (keyevent.keysym.sym)
    {
    case SDLK_SPACE:
      save_to_file();
      break;

    case SDLK_r:
      m_reset = true;
      break;

    case SDLK_j:
      m_bounces++;
      m_reset = true;
      break;

    case SDLK_k:
      if (m_bounces > 1)
      {
        m_bounces--;
        m_reset = true;
      }
      break;

    default:
      break;
    }
    break;
  }
  }
}

void Renderer::keyboard_state(const Uint8* state)
{
  const float speed = 10.0f * m_clock.delta;

  if (state[SDL_SCANCODE_W]) {
    m_camera.position += (m_camera.forward * speed);
    m_reset = true;
  }
  if (state[SDL_SCANCODE_S]) {
    m_camera.position -= (m_camera.forward * speed);
    m_reset = true;
  }
  if (state[SDL_SCANCODE_A]) {
    m_camera.position -= (m_camera.right * speed);
    m_reset = true;
  }
  if (state[SDL_SCANCODE_D]) {
    m_camera.position += (m_camera.right * speed);
    m_reset = true;
  }
  if (state[SDL_SCANCODE_E]) {
    m_camera.position += (m_camera.up * speed);
    m_reset = true;
  }
  if (state[SDL_SCANCODE_Q]) {
    m_camera.position -= (m_camera.up * speed);
    m_reset = true;
  }
}