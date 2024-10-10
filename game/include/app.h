#pragma once
#include <window.h>
#include <gfx/graphics.h>

namespace ut {
class app
{
public:
    app(size_t ui_thread) noexcept
        : ui_thread(ui_thread) { }

    w::action<void> init_async(uint32_t w, uint32_t height, bool fullscreen);

public:
private:
    size_t ui_thread;
    
    ut::window wnd;
    w::graphics gfx;
};
} // namespace ut