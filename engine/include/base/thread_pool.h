#pragma once
#include <thread_pool/thread_pool.h>
#include <wisdom/generated/api/api.h>
#include <coroutine>

namespace w::base {
class thread_pool;
thread_pool& get_thread_pool();

class thread_pool
{
    friend thread_pool& get_thread_pool();

public:
    static auto init_scoped() noexcept
    {
        struct scoped {
            ~scoped() noexcept
            {
                get_thread_pool().uninit();
            }
        };
        get_thread_pool().thread_pool.emplace(std::max(std::thread::hardware_concurrency(), 4u));
        return scoped{};
    }

public:
    void submit(std::coroutine_handle<> handle) noexcept
    {
        thread_pool->enqueue_detach([handle]() {
            handle.resume();
        });
    }
    void wait() noexcept
    {
        thread_pool->wait_for_tasks();
    }

private:
    void uninit() noexcept
    {
        thread_pool.reset();
    }

private:
    std::optional<dp::thread_pool<>> thread_pool;
};

} // namespace w::base