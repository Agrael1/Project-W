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
    w::error_message present() noexcept
    {
        return to_error(swap.Present());
    }
    w::error_message resize(uint32_t w, uint32_t h) noexcept
    {
        wis::DX12Info::Poll();
        return to_error(swap.Resize(w, h));
    }

public:
    wis::SwapChain swap;
};
} // namespace w
