#pragma once
#include <base/thread_pool.h>
#include <base/await_traits.h>
#include <coroutine>


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
    /// If an exception is thrown during the coroutine execution, it will be rethrown
    /// Executes eagerly, can be used as a top-level coroutine
    /// Cannot be awaited
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

    /// @brief Unique coroutine handle
    /// Manages the lifetime of the coroutine handle
    /// Behaves like a unique pointer
    struct unique_coroutine_handle
    {
    public:
        /// @brief Default constructor
        unique_coroutine_handle() noexcept
            : handle(nullptr)
        {}

        /// @brief Constructs the unique coroutine handle from the coroutine handle
        /// Takes ownership of the coroutine handle
        /// @param handle Coroutine handle
        unique_coroutine_handle(std::coroutine_handle<> handle) noexcept
            : handle(handle)
        {}

        unique_coroutine_handle(const unique_coroutine_handle&) = delete;
        unique_coroutine_handle& operator=(const unique_coroutine_handle&) = delete;

        unique_coroutine_handle(unique_coroutine_handle&& other) noexcept
            : handle(std::exchange(other.handle, nullptr))
        {}

        unique_coroutine_handle& operator=(unique_coroutine_handle&& other) noexcept
        {
            if (this != &other)
            {
                if (handle) handle.destroy();
                handle = std::exchange(other.handle, nullptr);
            }
            return *this;
        }

        ~unique_coroutine_handle() noexcept
        {
            if (handle) handle.destroy();
        }

        decltype(auto) operator->() const noexcept
        {
            return &handle;
        }

        operator bool() const noexcept
        {
            return handle != nullptr;
        }
    public:

        /// @brief Releases the ownership of the coroutine handle
        /// @return Coroutine handle
        std::coroutine_handle<> release() noexcept
        {
            return std::exchange(handle, nullptr);
        }

        /// @brief Resets the coroutine handle to the new value
        /// @param new_handle New coroutine handle
        void reset(std::coroutine_handle<> new_handle = nullptr) noexcept
        {
            if (handle) handle.destroy();
            handle = new_handle;
        }

        /// @brief Swaps the coroutine handles
        /// @param other Other coroutine handle
        void swap(unique_coroutine_handle& other) noexcept
        {
            std::swap(handle, other.handle);
        }

        /// @brief Gets the coroutine handle
        /// The ownership is not transferred
        /// @return Coroutine handle
        std::coroutine_handle<> get() const noexcept
        {
            return handle;
        }

        /// @brief Gets the coroutine handle for the specified promise type
        /// Does not transfer the ownership
        /// Does not check the promise type
        /// @tparam Promise Promise type
        /// @return Coroutine handle
        template<typename Promise> requires detail::is_promise<Promise>
        std::coroutine_handle<Promise> as() const noexcept
        {
            return std::coroutine_handle<Promise>::from_address(handle.address());
        }


        bool operator==(const unique_coroutine_handle& other) const noexcept
        {
            return handle == other.handle;
        }

        bool operator!=(const unique_coroutine_handle& other) const noexcept
        {
            return handle != other.handle;
        }
    private:
        std::coroutine_handle<> handle;
    };

    /// @brief Awaitable object that continues the coroutine execution 
    /// with provided continuation
    struct continued_awaitable
    {
        continued_awaitable() noexcept = default;
        continued_awaitable(std::coroutine_handle<> continuation) noexcept
            : continuation(continuation)
        {
        }

    public:

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_resume() const noexcept
        {
        }

        void await_suspend(std::coroutine_handle<> handle) const noexcept
        {
            if (continuation) continuation.resume();
        }

    private:
        std::coroutine_handle<> continuation;
    };
} // namespace w









namespace std {
    template<typename ...Args>
    struct coroutine_traits<w::fire_and_forget, Args...>
    {
        using promise_type = w::fire_and_forget;
    };
} // namespace std
