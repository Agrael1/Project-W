#pragma once
#include <sdl/window.h>

namespace ut {
class window
{
public:
    window() noexcept = default;
    window(w::sdl::window&& wnd) noexcept
        : wnd(std::move(wnd)) { }
    window(window&&) noexcept = default;
    window& operator=(window&&) noexcept = default;
    w::sdl::window& get() noexcept { return wnd; }
    const w::sdl::window& get() const noexcept { return wnd; }

public:

private:
    w::sdl::window wnd;
};

w::action<w::result<window>> create_window_async(const char* title, int w, int h, bool fullscreen);
}