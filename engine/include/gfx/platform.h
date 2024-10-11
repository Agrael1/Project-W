#pragma once
#include <wisdom/wisdom_platform.h>
#include <gfx/swapchain.h>
#include <base/result.h>

namespace w {
class graphics;
namespace sdl {
class window;
class sdl_factory;
} // namespace sdl

class platform_extension
{
    enum class selector {
        None,
        Windows,
        XCB,
        Wayland
    };

public:
    platform_extension() = default;
    platform_extension(const w::sdl::sdl_factory&);

public:
    w::result<w::swapchain>
    create_swapchain(w::graphics& gfx, const w::sdl::window& window) const noexcept;

public:
    wis::FactoryExtension* get() noexcept
    {
        return platform.get();
    }

private:
    selector current = selector::None;
    std::unique_ptr<wis::FactoryExtension> platform;
};
} // namespace w