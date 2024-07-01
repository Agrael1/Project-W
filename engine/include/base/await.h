#pragma once
#include <base/thread_pool.h>
#include <winrt/base.h>

winrt::fire_and_forget a;

namespace w {
    namespace detail {
        inline void resume_background(std::coroutine_handle<> handle) noexcept
        {
            base::get_thread_pool().submit(handle);
        }
    }

    // Resumes current coroutine on the background thread pool
    [[nodiscard]] inline auto resume_background() noexcept
    {
        struct awaitable
        {
            bool await_ready() const noexcept
            {
                return false;
            }

            void await_resume() const noexcept
            {
            }

            void await_suspend(std::coroutine_handle<> handle) const
            {
                detail::resume_background(handle);
            }
        };

        return awaitable{};
    }

    // fire_and_forget promise type
    struct fire_and_forget
    {
        fire_and_forget get_return_object() const noexcept
        {
            return{};
        }

        void return_void() const noexcept
        {
        }

        std::suspend_never initial_suspend() const noexcept
        {
            return{};
        }

        std::suspend_never final_suspend() const noexcept
        {
            return{};
        }

        void unhandled_exception() const noexcept
        {
            std::terminate();
        }
    };


} // namespace w::base

namespace std {
    template<typename ...Args>
    struct coroutine_traits<w::fire_and_forget, Args...>
    {
        using promise_type = w::fire_and_forget;
    };
} // namespace std