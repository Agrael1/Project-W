#include <platform/sdl/window.h>
#include <SDL3/SDL_video.h>

std::pair<int, int> w::sdl::window::pixel_size() const noexcept
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(wnd, &w, &h);
    return { w, h };
}

void w::sdl::window::destroy() noexcept
{
    if (wnd) {
        SDL_DestroyWindow(wnd);
    }
}
