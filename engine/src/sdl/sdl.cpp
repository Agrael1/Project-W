#include <sdl/sdl.h>
#include <SDL3/SDL_init.h>


w::sdl::scoped_sdl::~scoped_sdl() noexcept
{
    uninit_sdl();
}
w::result<w::sdl::scoped_sdl> w::sdl::init_sdl() noexcept
{
    if (!::SDL_Init(SDL_INIT_VIDEO)) {
        return { SDL_GetError(), w::error };
    }
    return {};
}

void w::sdl::uninit_sdl() noexcept
{
    SDL_Quit();
}
