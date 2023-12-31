#pragma once

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <string>

class Window
{
public:
    struct Clock
    {
        uint64_t last, now;
        float delta = 0.0f;
        void init();
        void tick();
    };

    Window(int width, int height, const std::string &name = "SDL Window");
    ~Window();
    void run();

protected:
    int m_width, m_height;
    int m_frames = 0;
    float m_time = 0.0f;
    bool m_quit = false;
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_context = nullptr;
    Clock m_clock;

    void poll_events();

    virtual void render(float dt);
    virtual void event(const SDL_Event &event);
    virtual void keyboard_state(const Uint8* state);
};