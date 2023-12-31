#include "window.h"

Window::Window(int width, int height, const std::string &name)
    : m_width(width), m_height(height)
{
  SDL_Init(SDL_INIT_VIDEO);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  m_context = SDL_GL_CreateContext(m_window);

  glewExperimental = GL_TRUE;
  glewInit();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
  ImGui_ImplOpenGL3_Init();

  glViewport(0, 0, width, height);
}

Window::~Window()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(m_context);
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}

void Window::run()
{
  m_clock.init();
  while (!m_quit)
  {
    m_frames++;
    m_clock.tick();
    poll_events();
    const Uint8* state = SDL_GetKeyboardState(NULL);
    keyboard_state(state);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(m_window);
    ImGui::NewFrame();

    render(m_clock.delta);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(m_window);
    m_time += m_clock.delta;
  }
}

void Window::poll_events()
{
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0)
  {
    if (e.type == SDL_QUIT) {
      m_quit = true;
      break;
    }

    if (!ImGui_ImplSDL2_ProcessEvent(&e)) {
      break;
    }

    event(e);
  }
}

void Window::render(float dt)
{
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Window::event(const SDL_Event &event)
{
}

void Window::keyboard_state(const Uint8 *state)
{
}

void Window::Clock::init()
{
  now = SDL_GetPerformanceCounter();
}

void Window::Clock::tick()
{
  last = now;
  now = SDL_GetPerformanceCounter();
  uint64_t freqency = SDL_GetPerformanceFrequency();
  delta = static_cast<float>(now - last) / static_cast<float>(freqency);
}
