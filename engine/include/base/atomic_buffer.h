#pragma once
#include <atomic>
#include <bit>

namespace w::base {
namespace detail {
template<size_t array_size, size_t elements_per_cache_line>
struct index_shuffle {
    static_assert(array_size > 0, "array_size must be greater than 0");
    static_assert(elements_per_cache_line > 0, "elements_per_cache_line must be greater than 0");
    static_assert(((elements_per_cache_line - 1) & elements_per_cache_line) == 0, "elements_per_cache_line must be power of two");

    static inline constexpr size_t bits = std::bit_ceil(elements_per_cache_line) - 1;
    static inline constexpr size_t min_size = 1u << (bits * 2);
    static inline constexpr size_t value = array_size < min_size ? 0 : bits;
};

template<size_t shuffle_bits>
constexpr size_t remap_index(size_t index) noexcept
{
    if constexpr (shuffle_bits == 0) {
        return index;
    } else {
        static constexpr size_t mix_mask = (1u << shuffle_bits) - 1;
        size_t mix = (index ^ (index >> shuffle_bits)) & mix_mask;
        return index ^ mix ^ (mix << shuffle_bits);
    }
}
} // namespace detail

template<class T, std::size_t Capacity>
    requires((Capacity & (Capacity - 1)) == 0 && std::atomic<T>::is_always_lock_free) // capacity has to be a power of two
struct atomic_buffer {
    static constexpr std::size_t mask = Capacity - 1;
    static constexpr T nullval = {};
    static constexpr int shuffle = detail::index_shuffle<Capacity, std::hardware_destructive_interference_size / sizeof(std::atomic<T>)>::value;

public:
    T get_unchecked(std::size_t idx, std::memory_order order) const noexcept { return _items[detail::remap_index<shuffle>(idx & mask)].load(order); }
    void put_unchecked(std::size_t idx, T value, std::memory_order order) noexcept { _items[detail::remap_index<shuffle>(idx & mask)].store(value, order); }

    static constexpr std::size_t capacity() noexcept { return Capacity; }
    static constexpr bool can_grow() noexcept { return false; }
    void grow(std::size_t, std::size_t) noexcept { }

private:
    std::atomic<T> _items[Capacity];
};
} // namespace w::base