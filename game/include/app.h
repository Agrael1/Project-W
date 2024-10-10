#pragma once
#include <base/tasks.h>
#include <window.h>

namespace ut {
class app
{
public:
    app(size_t ui_thread) noexcept
        : ui_thread(ui_thread) { }

    w::action<void> init_async(uint32_t w, uint32_t height, bool fullscreen);

public:
private:
    ut::window wnd;
    size_t ui_thread;
};
} // namespace ut