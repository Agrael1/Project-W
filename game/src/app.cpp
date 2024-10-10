#include <app.h>

w::action<void> ut::app::init_async()
{
    auto&&[r,w] = co_await ut::create_window_async("Game", 1920, 1080, false);
    wnd = std::move(w);
}
