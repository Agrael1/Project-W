#pragma once
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
} // namespace detail

constexpr inline vector operator+(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3] };
    } else {
        return _mm_add_ps(a, b);
    }
}
constexpr inline vector operator-(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3] };
    } else {
        return _mm_sub_ps(a, b);
    }
}
constexpr inline vector operator*(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3] };
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
        return { a[0] / b[0], a[1] / b[1], a[2] / b[2], a[3] / b[3] };
    } else {
        return _mm_div_ps(a, b);
    }
}
constexpr inline vector operator/(vector a, float b) noexcept
{
    return a / vector(b, broadcast);
}
constexpr inline vector operator|(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { float(uint32_t(a[0]) | uint32_t(b[0])),
                 float(uint32_t(a[1]) | uint32_t(b[1])),
                 float(uint32_t(a[2]) | uint32_t(b[2])),
                 float(uint32_t(a[3]) | uint32_t(b[3])) };
    } else {
        return _mm_or_ps(a, b);
    }
}
constexpr inline vector operator&(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { float(uint32_t(a[0]) & uint32_t(b[0])),
                 float(uint32_t(a[1]) & uint32_t(b[1])),
                 float(uint32_t(a[2]) & uint32_t(b[2])),
                 float(uint32_t(a[3]) & uint32_t(b[3])) };
    } else {
        return _mm_and_ps(a, b);
    }
}
constexpr inline vector operator^(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { float(uint32_t(a[0]) ^ uint32_t(b[0])),
                 float(uint32_t(a[1]) ^ uint32_t(b[1])),
                 float(uint32_t(a[2]) ^ uint32_t(b[2])),
                 float(uint32_t(a[3]) ^ uint32_t(b[3])) };
    } else {
        return _mm_xor_ps(a, b);
    }
}


template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline vector opposite(vector a) noexcept
{
    if (std::is_constant_evaluated()) {

        return {
            -a[0],
            Components > 1 ? -a[1] : a[1],
            Components > 2 ? -a[2] : a[2],
            Components > 3 ? -a[3] : a[3],
        };
    } else {
        constexpr auto eval = [](size_t thresh) constexpr { return Components > thresh ? -0.0f : 0.0f; };
        constexpr static float4 mask = { eval(0), eval(1), eval(2), eval(3) };
        return _mm_xor_ps(a, vector(mask));
    }
}

template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline vector dot(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        // clang-format off
        return { a[0] * b[0] 
               + a[1] * b[1] 
               + a[2] * b[2] 
               + a[3] * b[3], broadcast };
        // clang-format on
    } else {
        return _mm_dp_ps(a, b, dp_imm8<Components>());
    }
}

// only for 3D vectors
constexpr inline vector cross(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0], 0.0f };
    } else {
        constexpr uint4 mask = { 0, 0, 0, 0xFFFFFFFF };

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
    if (std::is_constant_evaluated()) {
        auto lsq = length_sq<Components>(a);
        float l = std::sqrt(lsq[0]);
        return { l, l, l, l };
    } else {
        return _mm_sqrt_ps(length_sq<Components>(a));
    }
}

template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline bool equal(vector a, vector b) noexcept
{
    if (std::is_constant_evaluated()) {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
    } else {
        constexpr auto value = detail::dp_imm8<Components>() & 0xF;
        return (_mm_movemask_ps(_mm_cmpeq_ps(a, b)) & value) == value;
    }
}

template<size_t Components = 3>
    requires(Components <= 4)
constexpr inline vector fmadd(vector a, vector b, vector c) noexcept
{
    if (std::is_constant_evaluated()) {
        return { a[0] * b[0] + c[0],
                 a[1] * b[1] + c[1],
                 a[2] * b[2] + c[2],
                 a[3] * b[3] + c[3] };
    } else {
        return _mm_fmadd_ps(a, b, c);
    }
}
} // namespace w::math