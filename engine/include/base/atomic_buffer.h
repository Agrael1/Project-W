#pragma once
#include <atomic>

namespace w::base {
template<class T, std::size_t Capacity>
    requires((Capacity & (Capacity - 1)) == 0 && std::atomic<T>::is_always_lock_free) // capacity has to be a power of two
struct atomic_buffer {
    static constexpr std::size_t mask = Capacity - 1;
    static constexpr T nullval = {};

public:
    std::size_t capacity() const noexcept { return Capacity; }

    [[nodiscard]] T get(std::size_t idx, std::memory_order order) noexcept { return std::atomic_exchange_explicit(_items + (idx & mask), nullval, order); }
    T get_unchecked(std::size_t idx, std::memory_order order) const noexcept { return _items[idx & mask].load(order); }

    bool put(std::size_t idx, T value, std::memory_order order) noexcept { return std::atomic_compare_exchange_strong_explicit(_items + (idx & mask), &nullval, value, order, std::memory_order_relaxed); }
    void put_unchecked(std::size_t idx, T value, std::memory_order order) noexcept { _items[idx & mask].store(value, order); }

    static constexpr bool can_grow() noexcept { return false; }
    void grow(std::size_t, std::size_t) noexcept { }

private:
    std::atomic<T> _items[Capacity];
};
} // namespace w::base