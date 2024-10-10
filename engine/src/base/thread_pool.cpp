#include <base/await.h>
#include <base/thread_pool.h>

void w::detail::resume_background(std::coroutine_handle<> handle) noexcept
{
    w::base::global_thread_pool_token::get_pool().submit(handle);
}

void w::detail::resume_affine(std::coroutine_handle<> handle, size_t thread_index) noexcept
{
    w::base::global_thread_pool_token::get_pool().submit_affine(handle, thread_index);
}

size_t w::detail::current() noexcept
{
    return w::base::global_thread_pool_token::get_pool().current_unit();
}