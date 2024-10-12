#include <app.h>
#include <SDL3/SDL_video.h>

w::action<void> ut::app::init_async(uint32_t width, uint32_t height, bool fullscreen)
{
    auto [r1, factory] = w::sdl::create_factory();
    if (!bool(r1)) {
        // log error
        co_return;
    }
    platform = w::platform_extension(factory);

    auto wnd_task = [this, &factory](bool fullscreen, uint32_t width, uint32_t height) -> w::action<void> {
        co_await w::resume_affine(ui_thread); // Resume on the UI thread

        auto [r2, window] = factory.create_window("Hello, World!", width, height, SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
        if (!bool(r2)) {
            // log error
            co_return;
        }
        // Construct the window
        std::construct_at(&wnd, std::move(window), std::move(factory));
    }(fullscreen, width, height);

    co_await w::when_all(gfx.init_async(platform), wnd_task);

    auto swap_task = [this]() -> w::action<void> {
        co_await w::resume_background();
        // Create the platform extension
        auto [r, swap] = platform.create_swapchain(gfx, wnd.get());
        if (!bool(r)) {
            // log error
            co_return;
        }
        // Construct the swapchain
        std::construct_at(&swapchain, std::move(swap));
    }();

    co_await swap_task;
}

w::action<int> ut::app::run_async()
{
    while (true)
    {
        if (process_events()) {
            co_return 0;
        }

        //co_await w::resume_background(); // Resume on the background thread
        // Render the frame


        swapchain.present(gfx);
    }
}

w::action<bool> ut::app::process_events_async()
{
    // do PMR here
    std::vector<w::action<void>> tasks;
    w::window_event event{};
    do {
        event = wnd.poll_events();
        switch (event) {
        case w::window_event::Quit:
            if (!tasks.empty())
                co_await w::when_all(std::span{ tasks });
            co_return true; // Quit the application
        case w::window_event::Resize:
            auto [w, h] = wnd.pixel_size();
            tasks.emplace_back(on_resize_async(w, h));
            break;
        }
    } while (event != w::window_event::NoEvent);

    if (!tasks.empty())
        co_await w::when_all(std::span{ tasks });
    co_return false;
}
bool ut::app::process_events()
{
    // do PMR here
    std::vector<w::action<void>> tasks;
    w::window_event event{};
    do {
        event = wnd.poll_events();
        switch (event) {
        case w::window_event::Quit:
            return true; // Quit the application
        case w::window_event::Resize:
            auto [w, h] = wnd.pixel_size();
            on_resize(w, h);
            break;
        }
    } while (event != w::window_event::NoEvent);
    return false;
}
w::action<void> ut::app::on_resize_async(int width, int height)
{
    co_await w::resume_background();
    auto e = swapchain.resize(uint32_t(width), uint32_t(height)); // Costly operation
    if (!bool(e)) {
        ; // log error
    }
}
void ut::app::on_resize(int width, int height)
{
    auto e = swapchain.resize(uint32_t(width), uint32_t(height)); // Costly operation
    if (!bool(e)) {
        ; // log error
    }
}
