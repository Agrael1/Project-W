#pragma once
#include <concepts>
#include <coroutine>

namespace w::detail
{
    template<typename T>
    concept is_awaiter = requires(T t)
    {
        { t.await_ready() } -> std::convertible_to<bool>;
        { t.await_suspend(std::coroutine_handle<>{}) };
        { t.await_resume() };
    };

    template<typename T>
    concept is_promise = requires(T t)
    {
        { t.get_return_object() };
        { t.initial_suspend() } -> is_awaiter;
        { t.final_suspend() } -> is_awaiter;
        { t.unhandled_exception() };
    };
}