#include <app.h>
#include <SDL3/SDL_video.h>

w::action<void> ut::app::init_async(uint32_t width, uint32_t height, bool fullscreen)
{
    auto wnd_task = [this](bool fullscreen, uint32_t width, uint32_t height) -> w::action<void> {
        co_await w::resume_affine(ui_thread); // Resume on the UI thread
        auto [r1, factory] = w::sdl::create_factory();
        if (!bool(r1)) {
            // log error
            co_return;
        }
        auto [r2, window] = factory.create_window("Hello, World!", width, height, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
        if (!bool(r2)) {
            // log error
            co_return;
        }
        // Construct the window
        std::construct_at(&wnd, std::move(window), std::move(factory));
    }(fullscreen, width, height);

    co_await wnd_task;

    co_return;
}
