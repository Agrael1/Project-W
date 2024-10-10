#pragma once
#include <base/atomic_buffer.h>
#include <optional>

namespace w::base {

// MPSC queue
template<class T, size_t buffer_size>
struct atomic_queue {
    using value_type = T;
    using container = w::base::atomic_buffer<T, buffer_size>;

    // could have used concepts, but this is more readable
    static_assert(buffer_size > 0 && (buffer_size & (buffer_size - 1)) == 0, "buffer_size must be a power of two");
    static_assert(std::atomic<T>::is_always_lock_free, "T must be lock-free");

public:
    atomic_queue() = default;

public:
    [[nodiscard]] bool try_push(value_type item) noexcept;
    [[nodiscard]] std::optional<value_type> try_pop() noexcept;
    [[nodiscard]] std::size_t capacity() const noexcept { return _items.capacity(); }
    [[nodiscard]] std::size_t size() const noexcept // useless, but for completeness
    {
        auto t = _top.load(std::memory_order_relaxed);
        return _bottom.load(std::memory_order_relaxed) - t;
    }

private:
    alignas(std::hardware_destructive_interference_size) std::atomic<std::size_t> _bottom{ 0 };
    alignas(std::hardware_destructive_interference_size) std::atomic<std::size_t> _top{ 0 };
    container _items;
};

// push may be called from only a single thread, so it can use relaxed synchronization
template<class T, size_t buffer_size>
bool atomic_queue<T, buffer_size>::try_push(value_type item) noexcept
{
    std::size_t b;
    do {
        b = _bottom.load(std::memory_order_relaxed);
        auto t = _top.load(std::memory_order_relaxed);
        auto size = b - t;
        if (size >= _items.capacity()) {
            return false;
        }
    } while (!_items.try_put(b, item, std::memory_order_relaxed));
    _bottom.store(b + 1, std::memory_order_release);
    return true;
}

// pop is only called from a single thread.
// edge case: steal vs pop, don't care if nullval is returned
template<class T, size_t buffer_size>
std::optional<typename atomic_queue<T, buffer_size>::value_type>
atomic_queue<T, buffer_size>::try_pop() noexcept
{
    auto b = _bottom.load(std::memory_order_relaxed);
    auto t = _top.load(std::memory_order_relaxed);
    if (b == t) {
        return std::nullopt;
    }
    // get from the top
    return _items.get(_top.fetch_sub(1, std::memory_order::relaxed), std::memory_order_relaxed);
}
} // namespace w::base