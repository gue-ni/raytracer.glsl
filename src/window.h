#pragma once

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
    bool m_quit = false;
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_context = nullptr;
    Clock m_clock;

    void poll_events();

    virtual void render(float dt);
    virtual void event(const SDL_Event &event);
};