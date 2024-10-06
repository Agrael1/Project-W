#pragma once
#include <thread_pool/thread_pool.h>
#include <base/async_queue.h>
#include <coroutine>
#include <optional>
#include <thread>

namespace w::base {
struct thread_unit {
    thread_unit() noexcept = default;
    thread_unit(auto thread_func) noexcept
        : thread(thread_func)
    {
    }

public:
    void request_stop() noexcept
    {
        thread.request_stop();
    }
    bool stop_requested() const noexcept
    {
        return thread.get_stop_token().stop_requested();
    }
    std::optional<std::coroutine_handle<>> pop_task() noexcept
    {
        do {
            auto val = queue.try_pop();
            if (val == std::nullopt) {
                return std::nullopt;
            }

            if (val && val.value()) {
                return val;
            }
            // else: try again
        } while (true);
    }
    void push_task(std::coroutine_handle<> handle) noexcept
    {
        while (!queue.try_push(handle)) {
            full.store(true, std::memory_order::relaxed);
            full.wait(true, std::memory_order::relaxed); // wait until popped or stolen
        }
    }

    void join() noexcept
    {
        thread.join();
    }

private:
    std::jthread thread;
    base::stealing_deque<std::coroutine_handle<>, 256> queue;
    std::atomic<bool> full = false;
};

class thread_pool
{
    friend struct thread_pool_token;

public:
    thread_pool() noexcept
    {
        const auto thread_count = std::thread::hardware_concurrency();
        units = std::make_unique<thread_unit[]>(thread_count);
        unit_count = thread_count;

        for (size_t i = 0; i < thread_count; ++i) {
            std::construct_at(units.get() + i, [this, i]() {
                index = i;
                thread_loop();
            });
        }
    }

public:
    void submit(std::coroutine_handle<> handle) noexcept
    {
        // always push to local queue
        units[index].push_task(handle);
    }
    void stop() noexcept
    {
        for (size_t i = 0; i < unit_count; ++i) {
            units[i].request_stop();
        }
    }

private:
    void thread_loop() noexcept
    {
        while (!units[index].stop_requested()) {
            auto task = units[index].pop_task(); // first try to pop from local queue
            if (task && task.value()) {
                task->resume();
                continue;
            }

            // if local queue is empty, try to steal from other queues
        }
    }

private:
    // std::optional<dp::thread_pool<>> thread_pool;
    thread_local static inline size_t index = 0;
    std::unique_ptr<thread_unit[]> units;
    size_t unit_count;
};

struct global_thread_pool_token {
    static global_thread_pool_token init_scoped() noexcept
    {
        return global_thread_pool_token();
    }
    static thread_pool& get_pool() noexcept
    {
        return *pool;
    }

private:
    global_thread_pool_token() noexcept
    {
        if (!pool) {
            pool.emplace();
        }
    }

public:
    ~global_thread_pool_token() noexcept
    {
        if (pool) {
            pool.reset();
        }
    }

private:
    static inline std::optional<thread_pool> pool;
};
} // namespace w::base
