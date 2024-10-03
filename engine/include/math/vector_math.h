#include <math/vector.h>

namespace w::math {
namespace detail {

template<int Components>
consteval inline int dp_imm8() noexcept
{
    static_assert(Components > 0 && Components <= 4, "Invalid number of components");
    // components transform to bit selector as follows:
    // 0 -> 0b0000, 1 -> 0b0001, 2 -> 0b0011, 3 -> 0b0111, 4 -> 0b1111

    int bit_selector = (1 << Components) - 1;
    return bit_selector | (bit_selector << 4); // use the same selector for all components in upper 4 bits
}
template<size_t Components>
constexpr float get_or(vector v, size_t component, float other) noexcept
{
    float arrdata[4] = { v.x, v.y, v.z, v.w };
    if constexpr (Components > 0) {
        return arrdata[component];
    } else {
        return other;
    }
}
} // namespace detail

constexpr inline vector operator+(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    } else {
        return _mm_add_ps(a, b);
    }
}
constexpr inline vector operator-(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
    } else {
        return _mm_sub_ps(a, b);
    }
}
constexpr inline vector operator*(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
    } else {
        return _mm_mul_ps(a, b);
    }
}
constexpr inline vector operator*(vector a, float b) noexcept
{
    return a * vector(b, broadcast);
}
constexpr inline vector operator*(float a, vector b) noexcept
{
    return vector(a, broadcast) * b;
}
constexpr inline vector operator/(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
    } else {
        return _mm_div_ps(a, b);
    }
}
constexpr inline vector operator/(vector a, float b) noexcept
{
    return a / vector(b, broadcast);
}

template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline vector opposite(vector a) noexcept
{
    if (std::is_constant_evaluated()) {
        return {
            -get_or<Components>(a, 0, -a.x), -get_or<Components>(a, 1, -a.y), -get_or<Components>(a, 2, -a.z), -get_or<Components>(a, 3, -a.w)
        };
    } else {
        constexpr auto eval = [](size_t thresh) constexpr { return Components > thresh ? -0.0f : 0.0f; };
        constexpr static float4 mask = { eval(0), eval(1), eval(2), eval(3) };
        return _mm_xor_ps(a, mask);
    }
}

template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline vector dot(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        // clang-format off
        return { get_or<Components>(a, 0, 0) * get_or<Components>(b, 0, 0) 
               + get_or<Components>(a, 1, 0) * get_or<Components>(b, 1, 0) 
               + get_or<Components>(a, 2, 0) * get_or<Components>(b, 2, 0) 
               + get_or<Components>(a, 3, 0) * get_or<Components>(b, 3, 0), broadcast };
        // clang-format on
    } else {
        return _mm_dp_ps(a, b, dp_imm8<Components>());
    }
}

// only for 3D vectors
constexpr inline vector cross(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0.0f };
    } else {
        constexpr static uint4 mask = { 0, 0, 0, 0xFFFFFFFF };
        
        vector t1 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)); // y,z,x,w
        vector t2 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2)); // z,x,y,w

        vector result = _mm_mul_ps(t1, t2); // y*z, z*x, x*y, w*w

        vector t3 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)); // z,x,y,w
        vector t4 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)); // y,z,x,w

        result = _mm_fnmadd_ps(t3, t4, result); // y*z - z*y, z*x - x*z, x*y - y*x, w*w

        return _mm_and_ps(result, vector(mask));
    }
}

template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline vector length_sq(vector a) noexcept
{
    return dot<Components>(a, a);
}

template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline vector length(vector a) noexcept
{
    return _mm_sqrt_ps(length_sq<Components>(a));
}
} // namespace w::math