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
    explicit vector(float x) noexcept
        : vector(x, 0, 0, 0)
    {
    }
    constexpr vector(__m128 o) noexcept
        : data(o)
    {
    }

    operator __m128() const noexcept
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

public:
    union {
        std::array<float, 4> arrdata;
        __m128 data{};
    };
};

struct float4 {
    constexpr float4() noexcept = default;
    constexpr float4(float x, float y, float z, float w) noexcept
        : x(x), y(y), z(z), w(w)
    {
    }
    constexpr float4(std::span<float, 4> xdata) noexcept
        : data{ xdata[0], xdata[1], xdata[2], xdata[3] }
    {
    }
    float4(vector v) noexcept
    {
        _mm_storeu_ps(data.data(), v);
    }
    operator vector() const noexcept
    {
        return _mm_loadu_ps(data.data());
    }

    // only constexpr if span constructor is used
    constexpr decltype(auto) operator[](size_t i) const noexcept
    {
        return data[i];
    }
    // only constexpr if span constructor is used
    constexpr decltype(auto) operator[](size_t i) noexcept
    {
        return data[i];
    }

public:
    union {
        std::array<float, 4> data{};
        struct {
            float x, y, z, w;
        };
        struct {
            float r, g, b, a;
        };
    };
};
struct alignas(alignof(__m128)) float4a : float4 {
    using float4::float4;
    float4a(vector v) noexcept
    {
        // aligned store
        _mm_store_ps(data.data(), v);
    }
    operator vector() const noexcept
    {
        // aligned load
        return _mm_load_ps(data.data());
    }
};

struct uint4 {
    constexpr uint4() noexcept = default;
    constexpr uint4(unsigned x, unsigned y, unsigned z, unsigned w) noexcept
        : x(x), y(y), z(z), w(w)
    {
    }
    uint4(vector v) noexcept
    {
        _mm_storeu_ps(reinterpret_cast<float*>(data.data()), v);
    }
    operator vector() const noexcept
    {
        return _mm_loadu_ps(reinterpret_cast<const float*>(data.data()));
    };

public:
    union {
        std::array<unsigned, 4> data{};
        struct {
            unsigned x, y, z, w;
        };
    };
};

struct alignas(alignof(__m128)) uint4a : uint4 {
    using uint4::uint4;
    uint4a(vector v) noexcept
    {
        // aligned store
        _mm_store_ps(reinterpret_cast<float*>(data.data()), v);
    }
    operator vector() const noexcept
    {
        // aligned load
        return _mm_load_ps(reinterpret_cast<const float*>(data.data()));
    }
};

struct float3 {
    constexpr float3() noexcept = default;
    constexpr float3(float x, float y, float z) noexcept
        : x(x), y(y), z(z)
    {
    }
    constexpr float3(std::span<float, 3> xdata) noexcept
        : data{ xdata[0], xdata[1], xdata[2] }
    {
    }
    float3(vector v) noexcept
    {
        _mm_store_sd(reinterpret_cast<double*>(data.data()), _mm_castps_pd(v));
        __m128 s_z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
        _mm_store_ss(&z, s_z);
    }
    operator vector() const noexcept
    {
        vector s_xy = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(data.data())));
        __m128 s_z = _mm_load_ss(&z);
        return _mm_movelh_ps(s_xy, s_z);
    }

    // only constexpr if span constructor is used
    constexpr decltype(auto) operator[](size_t i) const noexcept
    {
        return data[i];
    }
    // only constexpr if span constructor is used
    constexpr decltype(auto) operator[](size_t i) noexcept
    {
        return data[i];
    }

public:
    union {
        std::array<float, 3> data{};
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
    };
};

struct alignas(alignof(__m128)) float3a : float3 {
    using float3::float3;
    float3a(vector v) noexcept
    {
        // aligned store
        _mm_store_ps(data.data(), v);
    }
    operator vector() const noexcept
    {
        constexpr uint4a mask = { 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 }; // mask for xyz
        // aligned load
        vector s_xyzw = _mm_load_ps(data.data());
        return _mm_and_ps(s_xyzw, vector(mask));
    }
};

struct float2 {
    constexpr float2() noexcept = default;
    constexpr float2(float x, float y) noexcept
        : x(x), y(y)
    {
    }
    constexpr float2(std::span<float, 2> xdata) noexcept
        : data{ xdata[0], xdata[1] }
    {
    }
    float2(vector v) noexcept
    {
        _mm_store_sd(reinterpret_cast<double*>(data.data()), _mm_castps_pd(v));
    }
    operator vector() const noexcept
    {
        return _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(data.data())));
    }
    // only constexpr if span constructor is used
    constexpr decltype(auto) operator[](size_t i) const noexcept
    {
        return data[i];
    }
    // only constexpr if span constructor is used
    constexpr decltype(auto) operator[](size_t i) noexcept
    {
        return data[i];
    }

public:
    union {
        std::array<float, 2> data{};
        struct {
            float x, y;
        };
        struct {
            float r, g;
        };
    };
};

struct alignas(alignof(__m128)) float2a : float2 {
};

} // namespace w::math
