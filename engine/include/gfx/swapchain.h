#pragma once
#include <wisdom/wisdom.hpp>
#include <gfx/misc.h>

namespace w {
class swapchain
{
public:
    swapchain() noexcept = default;
    swapchain(wis::SwapChain&& swap) noexcept
        : swap(std::move(swap)) { }

public:
    void present() noexcept
    {
        swap.Present();
    }
    w::error_message resize(uint32_t w, uint32_t h) noexcept
    {
        return to_error(swap.Resize(w, h));
    }

private:
    wis::SwapChain swap;
};
} // namespace w
