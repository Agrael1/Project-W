#include <gfx/graphics.h>

w::action<void> w::graphics::init_async()
{
    co_await w::resume_background();

    auto [r1, factory] = wis::CreateFactory(true);
}