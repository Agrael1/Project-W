#pragma once
#include <window.h>

namespace ut {
class app
{
public:
    app() = default;
    w::action<void> init_async();

private:
    ut::window wnd;
};
}