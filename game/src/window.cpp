#include <window.h>
#include <SDL3/SDL_video.h>

w::action<w::result<ut::window>> ut::create_window_async(const char* title, int w, int h, bool fullscreen)
{
    auto [r, window] = co_await w::sdl::create_window_async(title, w, h, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    co_return w::result<ut::window>{ ut::window{ std::move(window) } };
}