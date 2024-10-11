#pragma once
#include <platform/sdl/sdl.h>

namespace ut {
class window
{
public:
    window() noexcept = default;
    window(w::sdl::window&& wnd, w::sdl::sdl_factory&& factory) noexcept
        : factory(std::move(factory)), wnd(std::move(wnd)) { }
    window(window&&) noexcept = default;
    window& operator=(window&&) noexcept = default;
    w::sdl::window& get() noexcept { return wnd; }
    const w::sdl::window& get() const noexcept { return wnd; }

public:
    std::pair<int, int> pixel_size() const noexcept
    {
        return wnd.pixel_size();
    }
    void set_title(const char* title) noexcept
    {
        wnd.set_title(title);
    }
    void set_fullscreen(bool fullscreen) noexcept
    {
        wnd.set_fullscreen(fullscreen);
    }
    w::window_event poll_events() noexcept
    {
        return wnd.poll_events();
    }


private:
    w::sdl::sdl_factory factory;
    w::sdl::window wnd;
};
}