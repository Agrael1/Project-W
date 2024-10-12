#pragma once
#include <window.h>
#include <gfx/graphics.h>
#include <gfx/platform.h>

namespace ut {
class app
{
public:
    app(size_t ui_thread = 0) noexcept
        : ui_thread(ui_thread) { }

public:
    w::action<void> init_async(uint32_t w, uint32_t height, bool fullscreen);
    void run();

private:
    w::action<bool> process_events_async(); // true if quit event was received
    w::action<void> on_resize_async(int width, int height);

    bool process_events(); // true if quit event was received
    void on_resize(int width, int height);
private:
    size_t ui_thread;
    
    ut::window wnd;
    w::graphics gfx;
    w::platform_extension platform;
    w::swapchain swapchain;
};
} // namespace ut