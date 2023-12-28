#include "renderer.h"
#include "gfx/gfx.h"
#include "kdtree.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <span>
#include <iostream>
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

using namespace gfx;
using namespace gfx::gl;

const std::string raytracer_shader = R"(
#include "raytracer.glsl"
)";

Renderer::Renderer(int width, int height) 
  : Window(width, height, "Pathtracer")
  , m_screen_shader(std::make_unique<ShaderProgram>(
      ShaderProgram::from_file("shaders/screen.vert"), 
      ShaderProgram::from_file("shaders/screen.frag")))
  , m_render_shader(std::make_unique<ShaderProgram>(ShaderProgram::from_file("shaders/raytracer.glsl")))
  , m_texture(std::make_unique<Texture>())
  , m_screen_quad_vao(std::make_unique<VertexArrayObject>())
  , m_screen_quad_vbo(std::make_unique<VertexBuffer>())
  , m_spheres(std::make_unique<ShaderStorageBuffer>())
  , m_materials(std::make_unique<ShaderStorageBuffer>())
  , m_vertices(std::make_unique<ShaderStorageBuffer>())
  , m_meshes(std::make_unique<ShaderStorageBuffer>())
  , m_kdtree(std::make_unique<ShaderStorageBuffer>())
  , m_camera(glm::vec3(0.0f, 0.0f, -35.0f), 33.0f)
{
  // setup screen quad
  const std::vector<glm::vec2> quad = {
      {-1, +1}, {0, 1},  // top left
      {-1, -1}, {0, 0},  // bottom left
      {+1, +1}, {1, 1},  // top right
      {+1, -1}, {1, 0},  // bottom right
  };

  m_screen_quad_vao->bind();
  m_screen_quad_vbo->bind();
  m_screen_quad_vbo->buffer_data(std::span(quad));

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec2), (void *)(sizeof(glm::vec2)));
  glEnableVertexAttribArray(1);

  m_screen_quad_vao->unbind();

  // setup render texture
  m_texture->bind();
  m_texture->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  m_texture->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  m_texture->set_parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  m_texture->set_parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
}

void Renderer::render(float dt)
{
  ImGuiWindowFlags window_flags = 0;

  ImGui::SetNextWindowPos(ImVec2(10, 10));

  ImGui::Begin("Options", nullptr, window_flags);
  ImGui::Text("FPS: %.2f", 1.0f / dt);
  ImGui::Text("Time: %.2f", m_time);
  ImGui::Checkbox("Use Envmap", &m_use_envmap);
  ImGui::Checkbox("Use DOF", &m_use_dof);
  ImGui::SliderInt("Bounces", &m_bounces, 1, 20);
  ImGui::SliderFloat("Aperture", &m_camera.aperture, 0.001f, 1.0f);
  ImGui::SliderFloat("Focal Length", &m_camera.focal_length, 0.001f, 50.0f);
  ImGui::SliderFloat("FOV", &m_camera.fov, 0.001f, 90.0f);
  if (ImGui::Button("Reset Buffer")) reset_buffer();
  if (ImGui::Button("Save Image")) save_to_file();
  ImGui::End();

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  m_spheres->bind_buffer_base(1);
  m_materials->bind_buffer_base(2);
  m_meshes->bind_buffer_base(3);
  m_vertices->bind_buffer_base(4);
  m_kdtree->bind_buffer_base(5);
  

  m_render_shader->bind();
  m_render_shader->set_uniform("u_time", m_time);
  m_render_shader->set_uniform("u_frames", m_frames);
  m_render_shader->set_uniform("u_samples", m_samples);
  m_render_shader->set_uniform("u_max_bounce", static_cast<unsigned int>(m_bounces));
  m_render_shader->set_uniform("u_background", m_background);
  m_render_shader->set_uniform("u_random", rand());

  if (m_envmap) {
    m_envmap->bind(3);
    m_screen_shader->set_uniform("u_envmap", 3);
    m_render_shader->set_uniform("u_use_envmap", m_use_envmap);
  } else {
    m_render_shader->set_uniform("u_use_envmap", false);
  }

  m_render_shader->set_uniform("u_use_dof", m_use_dof);
  m_render_shader->set_uniform("u_use_bvh", m_use_bvh);

  m_render_shader->set_uniform("u_camera_position", m_camera.position);
  m_render_shader->set_uniform("u_camera_fov", glm::radians(m_camera.fov));
  m_render_shader->set_uniform("u_camera_aperture", m_camera.aperture);
  m_render_shader->set_uniform("u_camera_focal_length", m_camera.focal_length);
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

  m_timer += dt;
  if (m_timer > 1)
  {
    m_timer = 0;
  }
}

void Renderer::set_spheres(const std::vector<Sphere>& spheres)
{
  m_spheres->bind();
  m_spheres->buffer_data(std::span(spheres));
}

void Renderer::set_materials(const std::vector<Material>& materials)
{
  m_materials->bind();
  m_materials->buffer_data(std::span(materials));
}

void Renderer::set_envmap(std::unique_ptr<CubemapTexture> envmap)
{
  m_envmap = std::move(envmap);
  m_envmap->bind();
  m_envmap->set_parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  m_envmap->set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  m_envmap->set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  m_envmap->set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  m_envmap->set_parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Renderer::set_vertices(const std::vector<glm::vec4>& vertices)
{
  auto triangles = to_triangles(vertices);
  m_vertices->bind();
  m_vertices->buffer_data(std::span(triangles));
}

void Renderer::set_meshes(const std::vector<Mesh>& meshes)
{
  m_meshes->bind();
  m_meshes->buffer_data(std::span(meshes));
}

void Renderer::set_nodes(const std::vector<KdNode>& nodes)
{
  m_kdtree->bind();
  m_kdtree->buffer_data(std::span(nodes));
}

void Renderer::save_to_file() const
{
  GLubyte* pixels = new GLubyte[m_width * m_height * 4]; 

  m_texture->bind();
  glGetTexImage(m_texture->target, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

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

glm::mat4 Renderer::transform(const glm::vec3& translate, const glm::vec3& scale, const glm::quat& rotate)
{
  glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
  glm::mat4 t = glm::translate(glm::mat4(1.0f), translate);
  glm::mat4 r = glm::mat4(rotate);
  return t * r * s;
}

std::vector<glm::vec4> Renderer::load_obj(const std::string &path)
{
  std::string warning, error;
  tinyobj::attrib_t attributes;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  const char* filename = path.c_str();

  if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, filename)) {
    std::cerr << "tinyobj: " << warning << error << std::endl;
    return {};
  } 

  printf("# of vertices  = %d\n", (int)(attributes.vertices.size()) / 3);
  printf("# of normals   = %d\n", (int)(attributes.normals.size()) / 3);
  printf("# of texcoords = %d\n", (int)(attributes.texcoords.size()) / 2);
  printf("# of materials = %d\n", (int)materials.size());
  printf("# of shapes    = %d\n", (int)shapes.size());

  std::vector<glm::vec4> vertices;

  for (size_t s = 0; s < shapes.size(); s++) {

    // Loop over faces(polygon)
    size_t index_offset = 0;

    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

      // Loop over vertices in the face.
      for (size_t v = 0; v < 3; v++) {

        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        glm::vec4 vertex;
        vertex.x = attributes.vertices[3 * idx.vertex_index + 0];
        vertex.y = attributes.vertices[3 * idx.vertex_index + 1];
        vertex.z = attributes.vertices[3 * idx.vertex_index + 2];
        vertex.w = 1.0f;

        vertices.push_back(vertex);
      }
      index_offset += 3;
    }
  }

  printf("# of triangles = %d\n",(int) vertices.size() / 3);
  return vertices;
}

void Renderer::reset_buffer()
{
  m_reset = true;
}

void Renderer::event(const SDL_Event &event)
{
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

    bool hover = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow);

    if (m_mousedown && !hover) {
      m_camera.yaw += delta_yaw;
      m_camera.pitch += delta_pitch;

      m_camera.forward = vector_from_spherical(m_camera.pitch, m_camera.yaw);
      m_camera.right = glm::normalize(glm::cross(m_camera.forward, glm::vec3(0.0f, 1.0f, 0.0f)));
      m_camera.up = glm::normalize(glm::cross(m_camera.right, m_camera.forward));

      reset_buffer();

    }
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
      reset_buffer();
      break;

    case SDLK_j:
      m_bounces++;
      reset_buffer();
      break;

    case SDLK_k:
      if (m_bounces > 1)
      {
        m_bounces--;
        reset_buffer();
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
    reset_buffer();
  }
  if (state[SDL_SCANCODE_S]) {
    m_camera.position -= (m_camera.forward * speed);
    reset_buffer();
  }
  if (state[SDL_SCANCODE_A]) {
    m_camera.position -= (m_camera.right * speed);
    reset_buffer();
  }
  if (state[SDL_SCANCODE_D]) {
    m_camera.position += (m_camera.right * speed);
    reset_buffer();
  }
  if (state[SDL_SCANCODE_E]) {
    m_camera.position += (m_camera.up * speed);
    reset_buffer();
  }
  if (state[SDL_SCANCODE_Q]) {
    m_camera.position -= (m_camera.up * speed);
    reset_buffer();
  }
}
