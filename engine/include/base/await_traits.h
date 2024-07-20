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
}