#pragma once
#include <math/quaternion.h>

namespace w::math {
// Creates a quaternion from an angle and an axis. The axis must be normalized.
// Unfortunatly, the compiler doesn't like the constexpr version of this function, since sin and cos are not constexpr.
inline quaternion from_angle_axis_normal(float angle, vector axis) noexcept
{
    constexpr vector select = vector(identity_mask[3]);
    float half_angle = angle * 0.5f;
    vector s = vector(std::sin(half_angle), broadcast);
    vector c = vector(std::cos(half_angle), broadcast);

    auto sel_ax = _mm_mul_ps(_mm_andnot_ps(select, axis), s);
    auto sel_ang = _mm_and_ps(select, c);
    return quaternion(sel_ax | sel_ang);
}

// Creates a quaternion from an angle and an axis. The axis is not required to be normalized.
inline quaternion from_angle_axis(float angle, vector axis) noexcept
{
    return from_angle_axis_normal(angle, normalize(axis));
}

// inefficient, but simple and easy to understand
inline angle_axis::operator quaternion() const noexcept
{
    vector axis = _mm_andnot_ps(vector(identity_mask[3]), *this);
    float angle = _mm_cvtss_f32(
            _mm_shuffle_ps(*this, *this, _MM_SHUFFLE(3, 3, 3, 3)));

    return from_angle_axis(angle, axis);
}

constexpr quaternion conjugate(quaternion q) noexcept
{
    if (std::is_constant_evaluated()) {
        return { -q[0], -q[1], -q[2], q[3] };
    } else {
        return _mm_xor_ps(q, vector(-0.0f, -0.0f, -0.0f, 0.0f));
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

quaternion slerp(quaternion a, quaternion b, float t) noexcept
{
    quaternion q = dot(a, b);
    float angle = std::acos(q[3]);
    float s1 = std::sin((1.0f - t) * angle);
    float s2 = std::sin(t * angle);
    return quaternion((a * s1 + b * s2) / std::sin(angle));
}

} // namespace w::math