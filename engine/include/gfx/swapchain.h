#pragma once
#include <wisdom/wisdom.hpp>

namespace w {
class swapchain
{
public:
    swapchain() noexcept = default;
    swapchain(wis::SwapChain&& swap) noexcept
        : swap(std::move(swap)) { }

public:

private:
    wis::SwapChain swap;
};
} // namespace w
