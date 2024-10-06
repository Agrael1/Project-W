#pragma once
#include <math/quaternion.h>

namespace w::math {
constexpr quaternion conjugate(quaternion q) noexcept
{
    if (std::is_constant_evaluated()) {
        return quaternion({ -q[0], -q[1], -q[2], q[3] });
    } else {
        return quaternion(q ^ vector(-0.0f, -0.0f, -0.0f, 0.0f));
    }
}
constexpr vector length(quaternion q) noexcept
{
    return length<4>(vector(q));
}
constexpr quaternion normalize(quaternion q) noexcept
{
    return quaternion(normalize<4>(vector(q)));
}
constexpr quaternion inverse(quaternion q) noexcept
{
    return quaternion(conjugate(q) / length(q));
}
constexpr quaternion dot(quaternion a, quaternion b) noexcept
{
    return quaternion(dot<4>(vector(a), vector(b)));
}

inline quaternion slerp(quaternion a, quaternion b, float t) noexcept
{
    quaternion q = dot(a, b);
    float angle = std::acos(q[3]);
    float s1 = std::sin((1.0f - t) * angle);
    float s2 = std::sin(t * angle);
    return quaternion((a * s1 + b * s2) / std::sin(angle));
}
inline quaternion pitch_yaw_roll(vector v) noexcept
{
    using enum detail::swizzle_mask;
    v = v * 0.5f;

    __m128 xcos{};
    vector xsin = _mm_sincos_ps(&xcos, v); // warning, svml

    //
    // cr *sp *cy + sr *cp *sy,
    // cr *cp *sy - sr *sp *cy,
    // sr *cp *cy - cr *sp *sy,
    // sr *sp *sy + cr *cp *cy

    vector m1 = _mm_shuffle_ps(xcos, xsin, detail::swizzle(z, z, z, z)); // cr cr sr sr
    vector m2 = _mm_shuffle_ps(xcos, xsin, detail::swizzle(y, y, y, y)); // cp cp sp sp
    vector m3 = _mm_shuffle_ps(xcos, xsin, detail::swizzle(x, x, x, x)); // cy cy sy sy

    m2 = _mm_shuffle_ps(m2, m2, detail::swizzle(w, x, x, w)); // sp cp cp sp 
    m3 = _mm_shuffle_ps(m3, m3, detail::swizzle(x, w, x, w)); // cy sy cy sy 

    vector rm1 = _mm_shuffle_ps(m1, m1, detail::swizzle(w, w, x, x)); // sr sr cr cr
    vector rm2 = _mm_shuffle_ps(m2, m2, detail::swizzle(y, x, x, y)); // cp sp sp cp
    vector rm3 = _mm_shuffle_ps(m3, m3, detail::swizzle(w, x, w, x)); // sy cy sy cy

    constexpr vector xormask = { 0.0f, -0.0f, -0.0f, 0.0f };
    vector r1 = (rm1 * rm2 * rm3) ^ xormask;
    return quaternion(_mm_fmadd_ps(m1 * m2, m3, r1));
}
} // namespace w::math