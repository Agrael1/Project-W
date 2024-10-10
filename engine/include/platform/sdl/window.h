#pragma once
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
    SDL_Window* get() const noexcept { return wnd; }

private:
    void destroy() noexcept;

private:
    SDL_Window* wnd;
};
} // namespace w::sdl