#include <platform/sdl/sdl.h>
#include <SDL3/SDL_init.h>

w::sdl::sdl_factory::~sdl_factory() noexcept
{
    if (initialized)
        ::SDL_Quit();
}

w::result<w::sdl::sdl_factory> w::sdl::create_factory() noexcept
{
    if (!::SDL_Init(SDL_INIT_VIDEO)) {
        return { SDL_GetError(), w::error };
    }
    return { w::sdl::sdl_factory{ true } };
}

w::result<w::sdl::window>
w::sdl::sdl_factory::create_window(const char* title, int w, int h, uint32_t flags) const
{
    SDL_Window* window = SDL_CreateWindow(title, w, h, flags);
    if (!window) {
        return w::result<w::sdl::window>{
            SDL_GetError(), w::error
        };
    }
    return w::result<w::sdl::window>{
        w::sdl::window{ window }
    };
}
