#pragma once
#include <platform/shared/window_event.h>
#include <utility>

struct SDL_Window;

namespace w::sdl {
class window
{
public:
    window() noexcept
        : wnd(nullptr) { }
    window(SDL_Window* wnd) noexcept
        : wnd(wnd) { }
    window(window&& o) noexcept
        : wnd(std::exchange(o.wnd, nullptr)) { }
    window& operator=(window&& o) noexcept
    {
        if (this != &o) {
            destroy();
            wnd = std::exchange(o.wnd, nullptr);
        }
        return *this;
    }
    ~window() noexcept
    {
        destroy();
    }

public:
    std::pair<int, int> pixel_size() const noexcept;
    void set_title(const char* title) noexcept;
    void set_fullscreen(bool fullscreen) noexcept;
    window_event poll_events() noexcept;

public:
    SDL_Window* get() const noexcept { return wnd; }

private:
    void destroy() noexcept;

private:
    SDL_Window* wnd;
    bool fullscreen = false;
};
} // namespace w::sdl