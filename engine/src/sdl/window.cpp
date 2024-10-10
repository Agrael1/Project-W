#include <sdl/window.h>
#include <SDL3/SDL_video.h>

w::action<w::result<w::sdl::window>> w::sdl::create_window_async(const char* title, int w, int h, uint32_t flags)
{
    SDL_Window* window = SDL_CreateWindow(title, w, h, flags);
    if (!window) {
        co_return w::result<w::sdl::window>{
            SDL_GetError(), w::error
        };
    }
    co_return w::result<w::sdl::window>{
        w::sdl::window{ window }
    };
}

void w::sdl::window::destroy() noexcept
{
    SDL_DestroyWindow(wnd);
}
