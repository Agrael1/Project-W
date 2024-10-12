#include <gfx/swapchain.h>
#include <gfx/graphics.h>

w::error_message w::swapchain::present(w::graphics& gfx) noexcept
{
    auto res = swap.Present();
    if (failed(res)) {
        return to_error(res);
    }

    const uint64_t vfence = fence_values[frame_index];
    gfx.get_main_queue().SignalQueue(fence, vfence);

    frame_index = swap.GetCurrentIndex();
    fence_values[frame_index] = vfence;

    if (fence.GetCompletedValue() < fence_values[frame_index])
        std::ignore = fence.Wait(fence_values[frame_index]);

    fence_values[frame_index] = vfence + 1;
}
