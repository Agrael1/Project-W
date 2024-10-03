#pragma once
#include <math/vector.h>

namespace w::math {
struct matrix {
public:
    constexpr matrix() noexcept = default;
    constexpr matrix(vector r0, vector r1, vector r2, vector r3) noexcept
        : r{ r0, r1, r2, r3 }
    {
    }

    constexpr decltype(auto) operator[](size_t i) const noexcept
    {
        return r[i];
    }
    constexpr decltype(auto) operator[](size_t i) noexcept
    {
        return r[i];
    }

public:
    vector r[4];
};

struct float4x4 {
public:
    constexpr float4x4() noexcept = default;
    constexpr float4x4(float4 r0, float4 r1, float4 r2, float4 r3) noexcept
        : r{ r0, r1, r2, r3 }
    {
    }
    constexpr float4x4(std::span<float, 16> xdata) noexcept
        : r{ { xdata[0], xdata[1], xdata[2], xdata[3] }, { xdata[4], xdata[5], xdata[6], xdata[7] }, { xdata[8], xdata[9], xdata[10], xdata[11] }, { xdata[12], xdata[13], xdata[14], xdata[15] } }
    {
    }
    constexpr float4x4(float x0, float y0, float z0, float w0,
                       float x1, float y1, float z1, float w1,
                       float x2, float y2, float z2, float w2,
                       float x3, float y3, float z3, float w3) noexcept
        : r{ { x0, y0, z0, w0 }, { x1, y1, z1, w1 }, { x2, y2, z2, w2 }, { x3, y3, z3, w3 } }
    {
    }
    constexpr float4x4(matrix m) noexcept
    {
        r[0] = float4(m[0]);
        r[1] = float4(m[1]);
        r[2] = float4(m[2]);
        r[3] = float4(m[3]);
    }

    // only constexpr if span constructor is used or if float4 with span constructor is used
    constexpr decltype(auto) operator[](size_t i) const noexcept
    {
        return r[i];
    }
    // only constexpr if span constructor is used or if float4 with span constructor is used
    constexpr decltype(auto) operator[](size_t i) noexcept
    {
        return r[i];
    }

public:
    float4 r[4];
};

// constants 
constexpr matrix identity = {
    { 1.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

} // namespace w::math