#include <base/await.h>
#include <base/thread_pool.h>

void w::detail::resume_background(std::coroutine_handle<> handle) noexcept
{
    w::base::global_thread_pool_token::get_pool().submit(handle);
}