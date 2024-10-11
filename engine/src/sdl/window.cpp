#include <platform/sdl/window.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

std::pair<int, int> w::sdl::window::pixel_size() const noexcept
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(wnd, &w, &h);
    return { w, h };
}

void w::sdl::window::set_title(const char* title) noexcept
{
    SDL_SetWindowTitle(wnd, title);
}

void w::sdl::window::set_fullscreen(bool fullscreen) noexcept
{
    if (this->fullscreen != fullscreen) {
        this->fullscreen = fullscreen;
        SDL_SetWindowFullscreen(wnd, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    }
}

w::window_event w::sdl::window::poll_events() noexcept
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (SDL_EventType(e.type)) {
        case SDL_EVENT_QUIT:
            return window_event::Quit;
        case SDL_EVENT_WINDOW_RESIZED:
            return window_event::Resize;
        }
    }
}

void w::sdl::window::destroy() noexcept
{
    if (wnd) {
        SDL_DestroyWindow(wnd);
    }
}
