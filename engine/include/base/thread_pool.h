#pragma once
#include <thread_pool/thread_pool.h>
#include <base/async_queue.h>
#include <coroutine>
#include <optional>
#include <thread>
#include <vector>

namespace w::base {
class thread_pool
{

private:
    std::optional<dp::thread_pool<>> thread_pool;

    std::vector<std::jthread> threads;


};
} // namespace w::base