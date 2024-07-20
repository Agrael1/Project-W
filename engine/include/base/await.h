#pragma once
#include <base/thread_pool.h>
#include <winrt/windows.foundation.h>
#include <exception>
#include <base/await_traits.h>

namespace w {
    namespace detail {
        /// @brief Resumes the coroutine on the background thread pool
        /// @param handle Coroutine handle to resume
        inline void resume_background(std::coroutine_handle<> handle) noexcept
        {
            base::get_thread_pool().submit(handle);
        }
    }

    /// @brief Resumes the coroutine on the background thread pool
    /// Current coroutine execution will be suspended and resumed on the background thread pool
    /// Suspension means that the control will be returned to the caller, and the coroutine will be resumed later
    /// @return Awaitable object
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

    /// @brief Fire and forget coroutine, does not return any value
    /// If an exception is thrown during the coroutine execution, it will be rethrown on final_suspend
    /// Executes eagerly, can be used as a top-level coroutine
    struct fire_and_forget
    {
        fire_and_forget get_return_object() const noexcept
        {
            return{};
        }

        void return_void() const noexcept { }

        std::suspend_never initial_suspend() const noexcept
        {
            return{};
        }

        std::suspend_never final_suspend() const noexcept
        {
            return {};
        }

        void unhandled_exception() const // throws exception
        {
            throw;
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
