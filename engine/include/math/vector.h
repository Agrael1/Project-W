#pragma once

// the library supports AVX2 and FMA3 by default
// ARM and ARM64 are not supported

#if defined(_MSC_VER) // MSVC only
#define WVECTORCALL __vectorcall
#else
#define WVECTORCALL
#endif

#include <immintrin.h>
#include <array>
#include <cmath>
#include <span>

namespace w::math {
struct broadcast_t {
};
constexpr broadcast_t broadcast{};

namespace detail {
template<typename T, size_t Components, size_t Alignment = alignof(T)>
struct array_storage {
    static_assert(Components > 0, "Invalid number of components");
    // check if alignment is power of 2
    static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");
    // check if valid simd type
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, unsigned> || std::is_same_v<T, double>, "Invalid simd type");

public:
    constexpr T get_or(size_t component, T other) const noexcept
    {
        return component < Components ? data[component] : other;
    }

public:
    // No constructor!
    constexpr T operator[](size_t i) const noexcept
    {
        return get_or(i, T{});
    }
    constexpr T& operator[](size_t i) noexcept
    {
        return data[i];
    }

public:
    constexpr T* begin() noexcept { return data.data(); }
    constexpr T* end() noexcept { return data.data() + Components; }

    constexpr const T* begin() const noexcept { return data.data(); }
    constexpr const T* end() const noexcept { return data.data() + Components; }

    constexpr size_t size() const noexcept { return Components; }

public:
    // x, y, z, w
    constexpr T x() const noexcept { return get_or(0, T{}); }
    constexpr T y() const noexcept { return get_or(1, T{}); }
    constexpr T z() const noexcept { return get_or(2, T{}); }
    constexpr T w() const noexcept { return get_or(3, T{}); }

    // r, g, b, a
    constexpr T r() const noexcept { return get_or(0, T{}); }
    constexpr T g() const noexcept { return get_or(1, T{}); }
    constexpr T b() const noexcept { return get_or(2, T{}); }
    constexpr T a() const noexcept { return get_or(3, T{}); }

public:
    alignas(Alignment) std::array<T, Components> data{};
};
} // namespace detail

struct vector {
    constexpr vector() noexcept = default;
    constexpr vector(float x, float y, float z, float w) noexcept
    {
        if (std::is_constant_evaluated()) {
            arrdata = { x, y, z, w };
        } else {
            data = _mm_set_ps(w, z, y, x);
        }
    }
    constexpr vector(float x, broadcast_t) noexcept
    {
        if (std::is_constant_evaluated()) {
            arrdata = { x, x, x, x };
        } else {
            data = _mm_broadcast_ss(&x);
        }
    }
    constexpr explicit vector(float x) noexcept
    {
        if (std::is_constant_evaluated()) {
            arrdata = { x, 0, 0, 0 };
        } else {
            data = _mm_load_ss(&x);
        }
    }
    constexpr vector(__m128 o) noexcept
        : data(o)
    {
    }

    constexpr operator __m128() const noexcept
    {
        return data;
    }
    constexpr explicit operator float() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return arrdata[0];
        } else {
            return _mm_cvtss_f32(data);
        }
    }
    constexpr decltype(auto) operator[](size_t i) const noexcept
    {
        return arrdata[i];
    }
    constexpr operator bool() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return arrdata[0] != 0 || arrdata[1] != 0 || arrdata[2] != 0 || arrdata[3] != 0;
        } else {
            return _mm_movemask_ps(data) != 0;
        }
    }

public:
    union {
        detail::array_storage<float, 4, alignof(__m128)> arrdata{};
        __m128 data;
    };
};

// integer vectors
//-------------------------------------------------------------------------
struct uint4 : detail::array_storage<unsigned, 4> {
    constexpr uint4() noexcept = default;
    constexpr uint4(unsigned x, unsigned y, unsigned z, unsigned w) noexcept
        : detail::array_storage<unsigned, 4>{ x, y, z, w }
    {
    }
    constexpr uint4(vector v) noexcept
    {
        if (std::is_constant_evaluated()) {
            data = { static_cast<unsigned>(v[0]), static_cast<unsigned>(v[1]), static_cast<unsigned>(v[2]), static_cast<unsigned>(v[3]) };
        } else {
            _mm_storeu_ps(reinterpret_cast<float*>(data.data()), v);
        }
    }
    constexpr operator vector() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return vector(static_cast<float>(x()), static_cast<float>(y()), static_cast<float>(z()), static_cast<float>(w()));
        } else {
            return _mm_loadu_ps(reinterpret_cast<const float*>(data.data()));
        }
    };
};
struct alignas(alignof(__m128)) uint4a : uint4 {
    using uint4::uint4;
    constexpr uint4a(vector v) noexcept
    {
        if (std::is_constant_evaluated()) {
            data = { static_cast<unsigned>(v[0]), static_cast<unsigned>(v[1]), static_cast<unsigned>(v[2]), static_cast<unsigned>(v[3]) };
        } else {
            // aligned store
            _mm_store_ps(reinterpret_cast<float*>(data.data()), v);
        }
    }
    constexpr operator vector() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return vector(static_cast<float>(x()), static_cast<float>(y()), static_cast<float>(z()), static_cast<float>(w()));
        } else {
            // aligned load
            return _mm_load_ps(reinterpret_cast<const float*>(data.data()));
        }
    }
};

// float vectors
//-------------------------------------------------------------------------
struct float4 : detail::array_storage<float, 4> {
    constexpr float4() noexcept = default;
    constexpr float4(float x, float y, float z, float w) noexcept
        : detail::array_storage<float, 4>{ x, y, z, w }
    {
    }
    constexpr float4(std::span<float, 4> xdata) noexcept
        : detail::array_storage<float, 4>{ xdata[0], xdata[1], xdata[2], xdata[3] }
    {
    }
    constexpr float4(vector v) noexcept
    {
        if (std::is_constant_evaluated()) {
            data = { v[0], v[1], v[2], v[3] };
        } else {
            _mm_storeu_ps(data.data(), v);
        }
    }
    constexpr operator vector() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return vector(data[0], data[1], data[2], data[3]);
        } else {
            return _mm_loadu_ps(data.data());
        }
    }
};
struct alignas(alignof(__m128)) float4a : float4 {
    using float4::float4;
    constexpr float4a(vector v) noexcept
    {
        if (std::is_constant_evaluated()) {
            data = { v[0], v[1], v[2], v[3] };
        } else {
            // aligned store
            _mm_store_ps(data.data(), v);
        }
    }
    constexpr operator vector() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return vector(data[0], data[1], data[2], data[3]);
        } else {
            // aligned load
            return _mm_load_ps(data.data());
        }
    }
};

struct float3 : detail::array_storage<float, 3> {
    constexpr float3() noexcept = default;
    constexpr float3(float x, float y, float z) noexcept
        : detail::array_storage<float, 3>{ x, y, z }
    {
    }
    constexpr float3(std::span<float, 3> xdata) noexcept
        : detail::array_storage<float, 3>{ xdata[0], xdata[1], xdata[2] }
    {
    }
    constexpr float3(vector v) noexcept
    {
        if (std::is_constant_evaluated()) {
            data = { v[0], v[1], v[2] };
        } else {
            _mm_store_sd(reinterpret_cast<double*>(data.data()), _mm_castps_pd(v));
            __m128 s_z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
            _mm_store_ss(&data[2], s_z);
        }
    }
    constexpr operator vector() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return vector(data[0], data[1], data[2], 0.0f);
        } else {
            __m128 s_xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(data.data())));
            __m128 s_z = _mm_load_ss(&data[2]);
            return _mm_movelh_ps(s_xy, s_z);
        }
    }
};
struct alignas(alignof(__m128)) float3a : float3 {
    using float3::float3;
    constexpr float3a(vector v) noexcept
    {
        if (std::is_constant_evaluated()) {
            data = { v[0], v[1], v[2] };
        } else {
            _mm_store_ps(data.data(), v);
        }
    }
    constexpr operator vector() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return vector(data[0], data[1], data[2], 0.0f);
        } else {
            // mask for xyz
            constexpr uint4a mask = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 }; // mask for xyz
            // aligned load
            vector s_xyzw = _mm_load_ps(data.data());
            return _mm_and_ps(s_xyzw, vector(mask));
        }
    }
};

struct float2 : detail::array_storage<float, 2> {
    constexpr float2() noexcept = default;
    constexpr float2(float x, float y) noexcept
        : detail::array_storage<float, 2>{ x, y }
    {
    }
    constexpr float2(std::span<float, 2> xdata) noexcept
        : detail::array_storage<float, 2>{ xdata[0], xdata[1] }
    {
    }
    constexpr float2(vector v) noexcept
    {
        if (std::is_constant_evaluated()) {
            data = { v[0], v[1] };
        } else {
            _mm_store_sd(reinterpret_cast<double*>(data.data()), _mm_castps_pd(v));
        }
    }
    constexpr operator vector() const noexcept
    {
        if (std::is_constant_evaluated()) {
            return vector(data[0], data[1], 0.0f, 0.0f);
        } else {
            return _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(data.data())));
        }
    }
};
struct alignas(alignof(__m128)) float2a : float2 {
};

// constatnts
//-------------------------------------------------------------------------
static inline constexpr uint4a identity_mask[4] = {
    { 0xffffffff, 0, 0, 0 },
    { 0, 0xffffffff, 0, 0 },
    { 0, 0, 0xffffffff, 0 },
    { 0, 0, 0, 0xffffffff }
};

} // namespace w::math
