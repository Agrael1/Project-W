#pragma once
#include <base/stealing_deque.h>
#include <base/atomic_queue.h>
#include <base/xoshiro.h>
#include <base/event_count.h>
#include <coroutine>
#include <optional>
#include <thread>
#include <immintrin.h>

namespace w::base {
struct thread_unit {
    static uint32_t generate_seed() noexcept
    {
        uint32_t a;
        _rdseed32_step(&a);
        return a;
    };
    thread_unit() noexcept = default;
    thread_unit(auto thread_func, bool affinity = false) noexcept
        : thread(thread_func)
        , rng(generate_seed())
        , affine_tasks(affinity ? std::make_unique<w::base::atomic_queue<std::coroutine_handle<>, 32>>() : nullptr)
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
    std::optional<std::coroutine_handle<>> steal_task() noexcept
    {
        do {
            auto val = queue.try_steal();
            if (val == std::nullopt) {
                return std::nullopt;
            }
            full.store(false, std::memory_order::relaxed);

            if (val && val.value()) {
                return val;
            }
            // else: try again
        } while (true);
    }
    std::optional<std::coroutine_handle<>> pop_task() noexcept
    {
        do {
            auto val = queue.try_pop();
            if (val == std::nullopt) {
                return std::nullopt;
            }
            full.store(false, std::memory_order::relaxed);
            full.notify_one();
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
    bool push_affine_task(std::coroutine_handle<> handle) noexcept
    {
        if (!affine_tasks) {
            return false; // no affine tasks
        }

        while (!affine_tasks->try_push(handle)) {
            full_affine.store(true, std::memory_order::relaxed);
            full_affine.wait(true, std::memory_order::relaxed); // wait until popped
        }
        return true;
    }
    std::optional<std::coroutine_handle<>> pop_affine_task() noexcept
    {
        if (!affine_tasks) {
            return std::nullopt; // no affine tasks
        }

        auto val = affine_tasks->try_pop();
        if (val == std::nullopt) {
            return std::nullopt;
        }
        full_affine.store(false, std::memory_order::relaxed);
        full_affine.notify_one();
        return val;
    }
    bool empty_queue() const noexcept
    {
        return queue.size() == 0;
    }

    void join() noexcept
    {
        thread.join();
    }

    size_t get_victim(size_t thread_count) noexcept
    {
        return rng.next() % thread_count;
    }

private:
    xoroshiro rng{ 0 };
    std::jthread thread;
    w::base::stealing_deque<std::coroutine_handle<>, 256> queue;
    std::unique_ptr<w::base::atomic_queue<std::coroutine_handle<>, 32>> affine_tasks;
    std::atomic<bool> full = false;
    std::atomic<bool> full_affine = false;
};

class thread_pool
{
    friend struct thread_pool_token;

public:
    thread_pool(uint32_t thread_count = std::thread::hardware_concurrency()) noexcept
    {
        units = std::make_unique<thread_unit[]>(thread_count);
        unit_count = thread_count;

        for (size_t i = 0; i < thread_count; ++i) {
            std::construct_at(units.get() + i, [this, i]() {
                index = i;
                thread_loop();
                // printf("%zd thread stopped\n", i);
            }, i == 0);
        }
    }

public:
    void submit(std::coroutine_handle<> handle) noexcept
    {
        // always push to local queue
        units[index].push_task(handle);
        notifier.notify_one();
    }
    size_t current_unit() const noexcept
    {
        return index;
    }

    void submit_affine(std::coroutine_handle<> handle, size_t thread_idx) noexcept
    {
        // always push to local queue
        units[thread_idx].push_affine_task(handle);
        notifier.notify_all();
    }
    void stop() noexcept
    {
        for (size_t i = 0; i < unit_count; ++i) {
            units[i].request_stop();
        }
        // printf("stopping\n");
        notifier.notify_all();
    }

private:
    void thread_loop() noexcept
    {
        // nullopt means it's time to stop
        while (auto a = wait_for_task()) {
            exploit_task(a.value());
        }
    }
    std::optional<std::coroutine_handle<>> explore_task() noexcept
    {
        size_t num_failed_steals = 0;
        size_t num_yields = 0;

        auto& unit = units[index];
        while (!unit.stop_requested()) {
            size_t victim = unit.get_victim(unit_count);
            auto task = units[victim].steal_task();

            if (task) {
                return task;
            }

            num_failed_steals++;
            if (num_failed_steals > unit_count) {
                std::this_thread::yield();
                num_failed_steals = 0;
                num_yields++;
                if (num_yields > unit_count) {
                    break;
                }
            }
        }
        return std::nullopt;
    }
    void exploit_task(std::coroutine_handle<> handle) noexcept
    {
        if (!active_threads.fetch_add(1, std::memory_order::relaxed) && !thief_threads.load(std::memory_order::relaxed)) {
            notifier.notify_one();
        }

        do {
            handle.resume();
            if (auto p = units[index].pop_task()) {
                handle = p.value();
            } else {
                break;
            }
        } while (true);
        active_threads.fetch_sub(1, std::memory_order::relaxed);
    }
    std::optional<std::coroutine_handle<>> wait_for_task() noexcept
    {
        auto& unit = units[index];

        do {
            if (auto task = unit.pop_affine_task()) {
                return task;
            }

            thief_threads.fetch_add(1, std::memory_order::relaxed);
        i_explore:
            if (auto task = explore_task()) {
                if (thief_threads.fetch_sub(1, std::memory_order::relaxed) == 1) {
                    notifier.notify_one();
                }
                return task;
            }

            if (!unit.empty_queue()) {
                auto task = unit.steal_task();
                if (task) {
                    if (thief_threads.fetch_sub(1, std::memory_order::relaxed) == 1) {
                        notifier.notify_one();
                    }
                    return task;
                } else {
                    goto i_explore;
                }
            }

            if (unit.stop_requested()) {
                thief_threads.fetch_sub(1, std::memory_order::relaxed);
                return std::nullopt;
            }

            if (thief_threads.fetch_sub(1, std::memory_order::relaxed) != 1 || active_threads.load() <= 0) {
                auto epoch = notifier.prepare_wait();
                printf("%zd waiting\n", index);
                notifier.wait(epoch);
                printf("%zd woke up\n", index);
            }
        } while (true);
    }

private:
    thread_local static inline size_t index = 0;
    std::unique_ptr<thread_unit[]> units;
    size_t unit_count;

    alignas(std::hardware_destructive_interference_size) w::base::event_count notifier;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> active_threads = 0;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> thief_threads = 0;
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
            pool.emplace(4);
        }
    }

public:
    ~global_thread_pool_token() noexcept
    {
        if (pool) {
            pool->stop();
            pool.reset();
        }
    }

private:
    static inline std::optional<thread_pool> pool;
};
} // namespace w::base
