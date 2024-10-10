#pragma once
#include <base/tasks.h>
#include <base/result.h>

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
    SDL_Window* get() const noexcept { return wnd; }

private:
    void destroy() noexcept;

private:
    SDL_Window* wnd;
};

w::action<w::result<window>> create_window_async(const char* title, int w, int h, uint32_t flags);
} // namespace w::sdl