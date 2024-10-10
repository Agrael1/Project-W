#pragma once
#include <wisdom/wisdom.hpp>
#include <base/tasks.h>

namespace w {
class graphics
{
public:
    w::action<void> init_async();

private:
    wis::Device device;
};
} // namespace w