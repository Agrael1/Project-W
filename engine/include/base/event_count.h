#pragma once
#include <atomic>

namespace w::base {
class event_count
{
public:
    void notify_one() noexcept
    {
        epoch.fetch_add(1, std::memory_order::acq_rel);
        epoch.notify_one();
    }
    void notify_all() noexcept
    {
        epoch.fetch_add(1, std::memory_order::acq_rel);
        epoch.notify_all();
    }
    uint32_t prepare_wait() noexcept
    {
        wcount.fetch_add(1, std::memory_order::acq_rel);
        return epoch.load(std::memory_order::acquire);
    }
    void cancel_wait() noexcept
    {
        wcount.fetch_sub(1, std::memory_order::seq_cst);
    }
    void wait(uint32_t old_epoch) noexcept
    {
        while (epoch.load(std::memory_order::acquire) == old_epoch) {
            epoch.wait(old_epoch);
        }
        wcount.fetch_sub(1, std::memory_order::seq_cst);
    }

private:
    std::atomic<uint32_t> wcount{ 0 };
    std::atomic<uint32_t> epoch{ 0 };
};
} // namespace w::base