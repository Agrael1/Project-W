#include <platform/sdl/window.h>
#include <SDL3/SDL_video.h>

void w::sdl::window::destroy() noexcept
{
    if (wnd) {
        SDL_DestroyWindow(wnd);
    }
}
