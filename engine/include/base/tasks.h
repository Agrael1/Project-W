#pragma once 
#include <base/await.h>

namespace w
{
    template<typename PromiseType>
    struct [[nodiscard]] coro_type
    {
    public:
        using promise_type = PromiseType;
        using return_type = typename promise_type::storage_type;
    public:
        coro_type() noexcept
            : coroutine(nullptr)
        {}

        explicit coro_type(std::coroutine_handle<> coroutine) noexcept
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
            if constexpr (std::is_void_v<return_type>)
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

    template<typename ResultType, typename CoroType, typename InitialSuspend = std::suspend_always>
        requires detail::is_awaiter<InitialSuspend>
    struct promise_base
    {
        using storage_type = std::conditional_t<std::is_reference_v<ResultType>, std::remove_reference_t<ResultType>*, ResultType>;
        using result_type = std::conditional_t<std::is_void_v<storage_type>, decltype(std::ignore), storage_type>;
        static inline constexpr bool is_movable = !std::is_reference_v<ResultType> && !std::is_void_v<ResultType> && !std::is_arithmetic_v<ResultType> && !std::is_pointer_v<ResultType>;

    public:
        promise_base() noexcept = default;
        promise_base(const promise_base&) = delete;
        promise_base(promise_base&&) = default;
        promise_base& operator=(const promise_base&) = delete;
        promise_base& operator=(promise_base&&) = default;

    public:
        InitialSuspend initial_suspend() const noexcept
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

        CoroType get_return_object() noexcept
        {
            return CoroType{ std::coroutine_handle<promise_base>::from_promise(*this) };
        }

        void set_continuation(std::coroutine_handle<> continuation) noexcept
        {
            this->continuation = continuation;
        }

        decltype(auto) get_continuation() const noexcept
        {
            return continuation;
        }

        decltype(auto) get_result() const noexcept requires !std::is_void_v<ResultType>
        {
            if constexpr (std::is_reference_v<ResultType>)
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

    protected:
        std::coroutine_handle<> continuation;
        result_type result{};
    };

    template<typename ResultType, typename CoroType>
    using task_promise_base = promise_base<ResultType, CoroType, std::suspend_always>;

    template<typename ResultType, typename CoroType>
    using action_promise_base = promise_base<ResultType, CoroType, std::suspend_never>;



    template<typename PromiseBase>
    struct promise_value : PromiseBase
    {
        using base_type = PromiseBase;
        using PromiseBase::PromiseBase;
        using storage_type = typename base_type::storage_type;

        template <typename U = storage_type> requires !std::is_void_v<storage_type>&& std::is_convertible_v<U&&, storage_type>
        void return_value(U&& value) noexcept
        {
            // Construct the value in place, avoids copy/move
            ::new (static_cast<void*>(std::addressof(base_type::result))) storage_type(std::forward<U>(value));
        }
    };

    template<typename PromiseBase>
    struct promise_void : PromiseBase
    {
        using PromiseBase::PromiseBase;
        void return_void() noexcept
        {
        }
    };

    template<typename T, typename CoroType>
    using task_promise = std::conditional_t<std::is_void_v<T>, promise_void<task_promise_base<void, CoroType>>, promise_value<task_promise_base<T, CoroType>>>;

    template<typename T, typename CoroType>
    using action_promise = std::conditional_t<std::is_void_v<T>, promise_void<action_promise_base<void, CoroType>>, promise_value<action_promise_base<T, CoroType>>>;

    /// @brief Lazy task coroutine
    /// @tparam ReturnType Value type
    template<typename ReturnType>
    struct task : coro_type<task_promise<ReturnType, task<ReturnType>>>
    {
        using coro_type<task_promise<ReturnType, task<ReturnType>>>::coro_type;
    };

    /// @brief Eager task coroutine
    /// @tparam ReturnType Value type
    template <typename ReturnType>
    struct action : coro_type<action_promise<ReturnType, action<ReturnType>>>
    {
        using coro_type<action_promise<ReturnType, action<ReturnType>>>::coro_type;
    };
}
