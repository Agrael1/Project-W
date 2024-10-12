#pragma once
#include <wisdom/wisdom.hpp>
#include <gfx/misc.h>

namespace w {
class graphics;
class swapchain final
{
public:
    swapchain() noexcept = default;
    swapchain(wis::SwapChain&& swap, wis::Fence&& fence) noexcept
        : swap(std::move(swap)), fence(std::move(fence)) { }

public:
    uint64_t get_frame_index() const noexcept
    {
        return frame_index;
    }
    w::error_message present(w::graphics& gfx) noexcept;
    w::error_message resize(uint32_t w, uint32_t h) noexcept
    {
        return to_error(swap.Resize(w, h));
    }

public:
    wis::SwapChain swap;
    wis::Fence fence;
    std::array<uint64_t, 3> fence_values{};
    uint64_t frame_index = 0;
};
} // namespace w
