#pragma once
#include <base/await.h>
#include <iostream>
#include <format>

namespace w {
template<typename PromiseType>
struct [[nodiscard]] coro_type {
public:
    using promise_type = PromiseType;
    using return_type = typename promise_type::storage_type;

public:
    coro_type() noexcept
        : coroutine(nullptr)
    {
    }

    explicit coro_type(std::coroutine_handle<> coroutine) noexcept
        : coroutine(coroutine)
    {
        std::cout << std::format("Coroutine {} created\n", typeid(PromiseType).name());
    }

    ~coro_type() noexcept
    {
        std::cout << std::format("Coroutine {} destroyed\n", typeid(PromiseType).name());

        if (await_ready())
            return;

        auto handle = coroutine.as<promise_type>();
        handle.promise().wait_finish_internal();
    }

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
        if constexpr (std::is_void_v<return_type>) {
            return;
        } else {
            auto handle = coroutine.as<promise_type>();
            return handle.promise().get_result();
        }
    }

    decltype(auto) get() noexcept
    {
        auto handle = coroutine.as<promise_type>();
        auto& promise = handle.promise();
        promise.wait_finish_internal();
        return await_resume();
    }

protected:
    unique_coroutine_handle coroutine;
};

template<typename ResultType, typename CoroType, typename InitialSuspend = std::suspend_always>
    requires detail::is_awaiter<InitialSuspend>
struct promise_base {
public:
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
        return {};
    }

    continued_awaitable final_suspend() const noexcept
    {
        auto awaitable = continued_awaitable{ get_continuation() };
        done.store(true, std::memory_order::release);
        if (waited) {
            done.notify_all(); // Probably expensive, is only done if someone is waiting
        }
        return awaitable;
    }

    void unhandled_exception() const
    {
        done.store(true, std::memory_order::release);
        if (waited) {
            done.notify_all();
        }
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

    decltype(auto) get_result() noexcept
        requires !std::is_void_v<ResultType>
    {
        if constexpr (std::is_reference_v<ResultType>) {
            return *result;
        } else if constexpr (is_movable) {
            return std::move(result);
        } else {
            return result;
        }
    }

    void wait_finish_internal() const noexcept
    {
        waited = true;
        done.wait(false, std::memory_order::acquire);
    }

protected:
    std::coroutine_handle<> continuation = std::noop_coroutine();
    result_type result{};

    mutable bool waited = false;
    mutable std::atomic_bool done{ false };
};

template<typename ResultType, typename CoroType>
using task_promise_base = promise_base<ResultType, CoroType, std::suspend_always>;

template<typename ResultType, typename CoroType>
using action_promise_base = promise_base<ResultType, CoroType, std::suspend_never>;

template<typename PromiseBase>
struct promise_value : PromiseBase {
    using base_type = PromiseBase;
    using PromiseBase::PromiseBase;
    using storage_type = typename base_type::storage_type;

    template<typename U = storage_type>
        requires !std::is_void_v<storage_type> && std::is_convertible_v<U&&, storage_type>
    void return_value(U && value) noexcept
    {
        // Construct the value in place, avoids copy/move
        ::new (static_cast<void*>(std::addressof(base_type::result))) storage_type(std::forward<U>(value));
    }
};

template<typename PromiseBase>
struct promise_void : PromiseBase {
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
struct task : coro_type<task_promise<ReturnType, task<ReturnType>>> {
    using coro_type<task_promise<ReturnType, task<ReturnType>>>::coro_type;
};

/// @brief Eager action coroutine. Very fast but must be waited upon in order for it to be destroyed, otherwise it may deadlock.
/// @tparam ReturnType Value type
template<typename ReturnType>
struct action : coro_type<action_promise<ReturnType, action<ReturnType>>> {
    using base = coro_type<action_promise<ReturnType, action<ReturnType>>>;
    using coro_type<action_promise<ReturnType, action<ReturnType>>>::coro_type;

    void await_suspend(
            std::coroutine_handle<> awaiting_coroutine) noexcept
    {
        auto handle = this->coroutine.as<base::promise_type>();
        handle.promise().set_continuation(awaiting_coroutine);
    }
};

/// @brief Wait for all tasks to finish. Evaluation is done from left to right.
/// @param args Tasks
/// @return Task that finishes when all tasks are done
template<typename ...Args>
task<void> when_all(Args&&... args)
{
    (co_await std::forward<Args>(args), ...);
}

} // namespace w
