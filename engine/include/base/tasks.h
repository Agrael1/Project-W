#pragma once 
#include <base/await.h>

namespace w
{
    template<typename T>
    struct [[nodiscard]] task;

    template<typename T>
    struct task_promise_base
    {
        using storage_type = std::conditional_t<std::is_reference_v<T>, std::remove_reference_t<T>*, T>;
        using result_type = std::conditional_t<std::is_void_v<storage_type>, decltype(std::ignore), storage_type>;
        static inline constexpr bool is_movable = !std::is_reference_v<T> && !std::is_void_v<T> && !std::is_arithmetic_v<T> && !std::is_pointer_v<T>;

    public:
        task_promise_base() noexcept = default;
        task_promise_base(const task_promise_base&) = delete;
        task_promise_base(task_promise_base&&) = default;
        task_promise_base& operator=(const task_promise_base&) = delete;
        task_promise_base& operator=(task_promise_base&&) = default;

    public:
        std::suspend_always initial_suspend() const noexcept
        {
            return{};
        }

        continued_awaitable final_suspend() const noexcept
        {
            return{};
        }

        void unhandled_exception() const
        {
            throw;
        }

        task<T> get_return_object() noexcept
        {
            return task<T>{ std::coroutine_handle<task_promise_base>::from_promise(*this) };
        }

        void set_continuation(std::coroutine_handle<> continuation) noexcept
        {
            this->continuation = continuation;
        }

        decltype(auto) get_continuation() const noexcept
        {
            return continuation;
        }

        decltype(auto) get_result() const noexcept requires !std::is_void_v<T>
        {
            if constexpr (std::is_reference_v<T>)
            {
                return *result.value;
            }
            else if constexpr(is_movable)
            {
                return std::move(result.value);
            }
            else
            {
                return result.value;
            }
        }

    private:
        std::coroutine_handle<> continuation;
        result_type result;
    };

    template<typename T>
    struct task_promise_value : task_promise_base<T>
    {
        using base_type = task_promise_base<T>;
        using task_promise_base<T>::task_promise_base;

        template <typename U = base_type::storage_type> requires !std::is_void_v<T>&& std::is_convertible_v<U&&, base_type::storage_type>
        void return_value(U&& value) noexcept
        {
            // Construct the value in place, avoids copy/move
            ::new (static_cast<void*>(std::addressof(base_type::result.value))) base_type::storage_type(std::forward<U>(value));
        }
    };

    struct task_promise_void : task_promise_base<void>
    {
        using task_promise_base<void>::task_promise_base;
        void return_void() noexcept
        {
        }
    };

    template<typename T>
    using task_promise = std::conditional_t<std::is_void_v<T>, task_promise_void, task_promise_value<T>>;


    /// @brief Lazy task coroutine, returns a value
    /// @tparam T Value type
    template<typename T>
    struct [[nodiscard]] task
    {
    public:
        using promise_type = task_promise<T>;
    public:
        task() noexcept
            : coroutine(nullptr)
        {}

        explicit task(std::coroutine_handle<> coroutine) noexcept
            : coroutine(coroutine)
        {}

    public:
        bool await_ready() const noexcept
        {
            return !coroutine || coroutine.get().done();
        }

        std::coroutine_handle<> await_suspend(
            std::coroutine_handle<> awaiting_coroutine) noexcept
        {
            auto handle = coroutine.as<promise_type>();
            handle.promise().set_continuation(awaiting_coroutine);
            return coroutine.get();
        }

        decltype(auto) await_resume() noexcept
        {
            if constexpr (std::is_void_v<T>)
            {
                return;
            }
            else if constexpr (promise_type::is_movable)
            {
                return std::move(coroutine->promise().result.value);
            }
            else
            {
                return coroutine->promise().result.value;
            }
        }

    private:
        unique_coroutine_handle coroutine;
    };

    using void_task = task<void>;
}

namespace std {
    template<typename T, typename ...Args>
    struct coroutine_traits<w::task<T>, Args...>
    {
        using promise_type = w::task_promise<T>;
    };
} // namespace std