#pragma once
#include <math/vector_math.h>
#include <math/matrix.h>
#include <numbers>
#include <algorithm>

namespace w::math {
struct quaternion : vector {
    constexpr quaternion() = default;
    explicit constexpr quaternion(vector v) noexcept
        : vector(v)
    {
    }
    constexpr operator matrix() const noexcept
    {
        vector q = static_cast<vector>(*this);
        if (std::is_constant_evaluated()) {
            return {
                vector(1 - 2.0f * (q[1] * q[1] + q[2] * q[2]), 2.0f * (q[0] * q[1] - q[2] * q[3]), 2.0f * (q[0] * q[2] + q[1] * q[3]), 0.0f),
                vector(2.0f * (q[0] * q[1] + q[2] * q[3]), 1.0f - 2.0f * (q[0] * q[0] + q[2] * q[2]), 2.0f * (q[1] * q[2] - q[0] * q[3]), 0.0f),
                vector(2.0f * (q[0] * q[2] - q[1] * q[3]), 2.0f * (q[1] * q[2] + q[0] * q[3]), 1.0f - 2.0f * (q[0] * q[0] + q[1] * q[1]), 0.0f),
                identity[3]
            };
        } else {
            using enum detail::swizzle_mask;
            constexpr vector onex3{ 1, 1, 1, 0 };
            vector _2q = q + q;
            vector q2 = q * _2q; // 2 * q * q

            vector v1 = _mm_shuffle_ps(q2, q2, detail::swizzle(y, x, x, w)); // 2yy, 2xx, 2xx
            vector v2 = _mm_shuffle_ps(q2, q2, detail::swizzle(z, z, y, w)); // 2zz, 2zz, 2yy
            vector sum1 = v1 + v2; // 2yy + 2zz, 2xx + 2zz, 2xx + 2yy
            vector r1 = neg_identity_mask[3] & (onex3 - sum1); // 1 - 2yy - 2zz, 1 - 2xx - 2zz, 1 - 2xx - 2yy, 0

            v1 = _mm_shuffle_ps(q, q, detail::swizzle(x, x, y, x));
            v2 = _mm_shuffle_ps(_2q, _2q, detail::swizzle(z, y, z, z));
            vector r2 = v1 * v2; // 2xz, 2xy, 2yz, 2xz

            v1 = _mm_shuffle_ps(q, q, detail::swizzle(w, w, w, w));
            v2 = _mm_shuffle_ps(_2q, _2q, detail::swizzle(y, z, x, y));
            vector r3 = v1 * v2; // 2yw, 2zw, 2xw, 2yw

            vector r4 = _mm_addsub_ps(r2, r3); // 2xz + 2yw, 2xy - 2zw, 2yz + 2xw, 2xz - 2yw
            r3 = r3 ^ vector(-0.0f, -0.0f, -0.0f, -0.0f); // negate the last component
            vector r5 = _mm_addsub_ps(r2, r3); // 2xz - 2yw, 2xy + 2zw, 2yz - 2xw, 2xz + 2yw

            v1 = _mm_shuffle_ps(r1, r4, detail::swizzle(x, w, y, x)); // 1 - 2yy - 2zz, 0, 2xy - 2zw, 2xz + 2yw
            v2 = _mm_shuffle_ps(r1, r5, detail::swizzle(y, w, z, y)); // 1 - 2xx - 2zz, 0, 2yz - 2xw, 2xy + 2zw
            r1 = _mm_shuffle_ps(r1, r4, detail::swizzle(z, w, w, z)); // 1 - 2xx - 2yy, 0, 2xz - 2yw, 2yz + 2xw

            return matrix(
                    _mm_shuffle_ps(v1, v1, detail::swizzle(x, z, w, y)), // 1 - 2yy - 2zz, 2xy - 2zw, 2xz + 2yw, 0
                    _mm_shuffle_ps(v2, v2, detail::swizzle(w, x, z, y)), // 2xy + 2zw, 1 - 2xx - 2zz, 2yz - 2xw, 0
                    _mm_shuffle_ps(r1, r1, detail::swizzle(z, w, x, y)), // 2xz - 2yw, 2yz + 2xw, 1 - 2xx - 2yy, 0
                    identity_mask[3]);
        }
    }

public:
    // Creates a quaternion from an angle and an axis. The axis must be normalized.
    // Unfortunatly, the compiler doesn't like the constexpr version of this function, since sin and cos are not constexpr.
    static inline quaternion from_angle_axis_normal(float angle, vector axis) noexcept
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
    static inline quaternion from_angle_axis(float angle, vector axis) noexcept
    {
        return from_angle_axis_normal(angle, normalize(axis));
    }
};

// storage type for quaternion
template<typename T = float4a>
    requires(std::same_as<T, float4> || std::same_as<T, float4a>)
struct quat : public T {
    using T::T;
};

// storage type for angle axis representation
template<typename T = float4a>
    requires(std::same_as<T, float4> || std::same_as<T, float4a>)
struct angle_axis : public T {
    using underlying_t = T;

    constexpr angle_axis() = default;
    constexpr angle_axis(float angle, vector axis) noexcept
        : underlying_t(axis)
    {
        this->data[3] = angle;
    }
    explicit angle_axis(quaternion q) noexcept
    {
        // extract angle
        float angle = std::acosf(_mm_cvtss_f32(_mm_shuffle_ps(q, q, _MM_SHUFFLE(3, 3, 3, 3))));
        // extract axis
        vector axis = _mm_andnot_ps(vector(identity_mask[3]), q);
        // divide by sin(angle) to normalize
        vector sin_angle = vector(std::sinf(angle), broadcast);
        static_cast<underlying_t&>(*this) = vector(_mm_div_ps(axis, sin_angle));
    }

public:
    operator quaternion() const noexcept
    {
        vector loaded = static_cast<vector>(*this);
        vector axis = _mm_andnot_ps(vector(identity_mask[3]), loaded);
        float angle = _mm_cvtss_f32(
                _mm_shuffle_ps(loaded, loaded, _MM_SHUFFLE(3, 3, 3, 3)));

        return quaternion::from_angle_axis(angle, axis);
    }
};

template<typename T = float4a>
    requires(std::same_as<T, float3> || std::same_as<T, float3a>)
struct spherical : public T {
    constexpr spherical() = default;
    constexpr spherical(float phi, float theta, float radius) noexcept
        : T{ phi, theta, radius }
    {
    }
    explicit spherical(vector coord3d) noexcept
    {
        float radius = length(coord3d);

        auto xval = _mm_shuffle_ps(coord3d, coord3d, _MM_SHUFFLE(0, 0, 0, 0));
        auto yval = _mm_shuffle_ps(coord3d, coord3d, _MM_SHUFFLE(1, 1, 1, 1));
        auto zval = _mm_shuffle_ps(coord3d, coord3d, _MM_SHUFFLE(2, 2, 2, 2));
        auto len2 = length<2>(coord3d);

        float phi = std::atanf(_mm_cvtss_f32(yval) / _mm_cvtss_f32(xval));
        float theta = std::acosf(zval / radius);
        this->data[0] = phi;
        this->data[1] = theta;
        this->data[2] = radius;
    }
    vector to_cartesian() const noexcept
    {
        using enum detail::swizzle_mask;
        float phi = this->data[0];
        float theta = this->data[1];
        float radius = this->data[2];
        auto v1 = vector(float4a{ std::sinf(theta), std::cosf(theta), std::sinf(phi), std::cosf(phi) });
        auto vradius = vector(radius, broadcast);

        auto shuf1 = _mm_shuffle_ps(v1, v1, detail::swizzle(x, x, y, w)); // sin(theta), sin(theta), cos(theta), ~
        auto shuf2 = _mm_shuffle_ps(v1, identity[0], detail::swizzle(w, z, x, z)); // cos(phi), sin(phi), 1, ~
        return vradius * shuf1 * shuf2;
    }
    void normalize() noexcept
    {
        this->data[0] = std::clamp(this->data[0], 0, 2 * std::numbers::pi_v<float>);
        this->data[1] = std::clamp(this->data[1], 0, 2 * std::numbers::pi_v<float>);
    }
};

template<typename T = float2>
    requires(std::same_as<T, float2> || std::same_as<T, float2a>)
struct polar : public T {
    constexpr polar() = default;
    constexpr polar(float angle, float radius) noexcept
        : T{ angle, radius }
    {
    }
    explicit polar(vector coord2d) noexcept
    {
        float radius = length(coord2d);
        float angle = std::atan2f(coord2d[1], coord2d[0]);
        this->data[0] = angle;
        this->data[1] = radius;
    }
    vector to_cartesian() const noexcept
    {
        float angle = this->data[0];
        float radius = this->data[1];
        return vector(radius * std::cosf(angle), radius * std::sinf(angle));
    }
    void normalize() noexcept
    {
        this->data[0] = std::clamp(this->data[0], 0, 2 * std::numbers::pi_v<float>);
    }
};

} // namespace w::math