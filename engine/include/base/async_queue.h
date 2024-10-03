// Based on: https://github.com/mpoeter/xenium/blob/master/xenium/chase_work_stealing_deque.hpp
#pragma once
#include <base/atomic_buffer.h>
#include <optional>

namespace w::base {
template<class T, size_t buffer_size>
struct stealing_deque {
    using value_type = T;
    using container = w::base::atomic_buffer<T, buffer_size>;

    // could have used concepts, but this is more readable
    static_assert(buffer_size > 0 && (buffer_size & (buffer_size - 1)) == 0, "buffer_size must be a power of two");
    static_assert(std::atomic<T>::is_always_lock_free, "T must be lock-free");

public:
    stealing_deque() = default;

public:
    [[nodiscard]] bool try_push(value_type item) noexcept;
    [[nodiscard]] std::optional<value_type> try_pop() noexcept;
    [[nodiscard]] std::optional<value_type> try_steal() noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept { return _items.capacity(); }
    [[nodiscard]] std::size_t size() const noexcept // useless, but for completeness
    {
        auto t = _top.load(std::memory_order_relaxed);
        return _bottom.load(std::memory_order_relaxed) - t;
    }

private:
    alignas(std::hardware_destructive_interference_size * 2) std::atomic<std::size_t> _bottom{ 0 };
    alignas(std::hardware_destructive_interference_size * 2) std::atomic<std::size_t> _top{ 0 };
    container _items;
};

// push may be called from only a single thread, so it can use relaxed synchronization
template<class T, size_t buffer_size>
bool stealing_deque<T, buffer_size>::try_push(value_type item) noexcept
{
    auto b = _bottom.load(std::memory_order_relaxed);
    auto t = _top.load(std::memory_order_relaxed);
    auto size = b - t;
    if (size >= _items.capacity()) {
        return false;
    }

    _items.put_unchecked(b, item, std::memory_order_relaxed);
    _bottom.store(b + 1, std::memory_order_release);
    return true;
}

// pop is only called from a single thread.
// edge case: steal vs pop, don't care if nullval is returned
template<class T, size_t buffer_size>
std::optional<typename stealing_deque<T, buffer_size>::value_type>
stealing_deque<T, buffer_size>::try_pop() noexcept
{
    auto b = _bottom.load(std::memory_order_relaxed);
    auto t = _top.load(std::memory_order_relaxed);
    if (b == t) {
        return false;
    }

    --b;
    _bottom.store(b, std::memory_order_seq_cst);

    value_type item = _items.get_unchecked(b, std::memory_order_relaxed);
    if (b > t) {
        return item;
    }

    if (b == t) {
        if (_top.compare_exchange_strong(t, t + 1, std::memory_order_relaxed)) {
            _bottom.store(t + 1, std::memory_order_relaxed);
            return item;
        }
    }
    _bottom.store(t, std::memory_order_relaxed);
    return _items.nullval;
}

template<class T, size_t buffer_size>
std::optional<typename stealing_deque<T, buffer_size>::value_type>
stealing_deque<T, buffer_size>::try_steal() noexcept
{
    auto t = _top.load(std::memory_order_relaxed);
    auto b = _bottom.load(std::memory_order_seq_cst);
    auto size = static_cast<std::intptr_t>(b) - static_cast<std::intptr_t>(t);
    if (size <= 0) {
        return std::nullopt;
    }

    auto item = _items.get_unchecked(t, std::memory_order_relaxed);
    if (!_top.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) {
        return _items.nullval;
    }
    return item;
}
} // namespace w::base
