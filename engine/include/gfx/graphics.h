#pragma once
#include <wisdom/wisdom.hpp>
#include <wisdom/wisdom_extended_allocation.h>
#include <base/tasks.h>

namespace w {
class platform_extension;
class graphics
{
public:
    w::action<void> init_async(platform_extension& ext);

public:
    const wis::Device& get_device() const noexcept
    {
        return device;
    }
    wis::CommandQueue& get_main_queue() noexcept
    {
        return main_queue;
    }
    const wis::ResourceAllocator& get_allocator() const noexcept
    {
        return allocator;
    }
    const wis::ExtendedAllocation& get_extended_allocator() const noexcept
    {
        return extended_alloc;
    }
    auto create_fence() const noexcept
    {
        return device.CreateFence();
    }

private:
    void create_device(const wis::Factory& factory);

private:
#ifndef NDEBUG
    wis::DebugMessenger debug_info;
#endif
    wis::Device device;
    wis::CommandQueue main_queue;
    wis::ExtendedAllocation extended_alloc;
    wis::ResourceAllocator allocator;
};
} // namespace w