#pragma once
#include <thread_pool/thread_pool.h>
#include <coroutine>
#include <optional>

namespace w::base {
class thread_pool
{

private:
    std::optional<dp::thread_pool<>> thread_pool;
};
} // namespace w::base