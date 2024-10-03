#pragma once
#include <math/matrix.h>
#include <math/vector_math.h>

namespace w::math {

constexpr matrix multiply(const matrix& a, const matrix& b) noexcept
{
    if (std::is_constant_evaluated()) {
        auto eval_row = [&](size_t index) {
            float x = a[index][0];
            float y = a[index][1];
            float z = a[index][2];
            float w = a[index][3];

            return vector{
                (b[0][0] * x) + (b[1][0] * y) + (b[2][0] * z) + (b[3][0] * w),
                (b[0][1] * x) + (b[1][1] * y) + (b[2][1] * z) + (b[3][1] * w),
                (b[0][2] * x) + (b[1][2] * y) + (b[2][2] * z) + (b[3][2] * w),
                (b[0][3] * x) + (b[1][3] * y) + (b[2][3] * z) + (b[3][3] * w),
            };
        };

        return {
            eval_row(0),
            eval_row(1),
            eval_row(2),
            eval_row(3)
        };
    } else {
        __m256 t0 = _mm256_castps128_ps256(a.r[0]);
        t0 = _mm256_insertf128_ps(t0, a.r[1], 1);
        __m256 t1 = _mm256_castps128_ps256(a.r[2]);
        t1 = _mm256_insertf128_ps(t1, a.r[3], 1);

        __m256 u0 = _mm256_castps128_ps256(b.r[0]);
        u0 = _mm256_insertf128_ps(u0, b.r[1], 1);
        __m256 u1 = _mm256_castps128_ps256(b.r[2]);
        u1 = _mm256_insertf128_ps(u1, b.r[3], 1);

        __m256 a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(0, 0, 0, 0));
        __m256 a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(0, 0, 0, 0));
        __m256 b0 = _mm256_permute2f128_ps(u0, u0, 0x00);
        __m256 c0 = _mm256_mul_ps(a0, b0);
        __m256 c1 = _mm256_mul_ps(a1, b0);

        a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(1, 1, 1, 1));
        a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 1, 1, 1));
        b0 = _mm256_permute2f128_ps(u0, u0, 0x11);
        __m256 c2 = _mm256_fmadd_ps(a0, b0, c0);
        __m256 c3 = _mm256_fmadd_ps(a1, b0, c1);

        a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 2, 2, 2));
        a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2));
        __m256 b1 = _mm256_permute2f128_ps(u1, u1, 0x00);
        __m256 c4 = _mm256_mul_ps(a0, b1);
        __m256 c5 = _mm256_mul_ps(a1, b1);

        a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(3, 3, 3, 3));
        a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 3, 3, 3));
        b1 = _mm256_permute2f128_ps(u1, u1, 0x11);
        __m256 c6 = _mm256_fmadd_ps(a0, b1, c4);
        __m256 c7 = _mm256_fmadd_ps(a1, b1, c5);

        t0 = _mm256_add_ps(c2, c6);
        t1 = _mm256_add_ps(c3, c7);

        return {
            _mm256_castps256_ps128(t0),
            _mm256_extractf128_ps(t0, 1),
            _mm256_castps256_ps128(t1),
            _mm256_extractf128_ps(t1, 1)
        };
    }
}
template<size_t Components = 3>
    requires(Components <= 4)
constexpr vector transform(const matrix& matr, vector v) noexcept
{
    if (std::is_constant_evaluated()) {
        constexpr auto last_component = Components - 1;
        vector result = { v[last_component] * matr[last_component][0],
                          v[last_component] * matr[last_component][1],
                          v[last_component] * matr[last_component][2],
                          v[last_component] * matr[last_component][3] };
        if constexpr (Components == 4)
            result = result + v[last_component] * matr[last_component];
        else
            result = result + matr[3];

        for (size_t i = 0; i < last_component; ++i) {
            result = result + v[i] * matr[i];
        }
        return result;
    } else {
        constexpr auto last_component = Components - 1;
        vector result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(last_component, last_component, last_component, last_component)); // W

        if constexpr (Components == 4)
            result = _mm_mul_ps(result, matr[last_component]);
        else
            result = fmadd(result, matr[last_component], matr[3]);

        auto a = [&]<size_t index> {
            vector vTemp = _mm_shuffle_ps(v, v, _MM_SHUFFLE(index, index, index, index));
            result = fmadd(vTemp, matr[index], result);
        };

        if constexpr (Components == 4) {
            a.operator()<2>();
        }
        if constexpr (Components == 3) {
            a.operator()<1>();
        }
        a.operator()<0>();
        // use fold expression
        return result;
    }
}

constexpr matrix operator*(const matrix& a, const matrix& b) noexcept
{
    return multiply(a, b);
}
constexpr vector operator*(const matrix& a, vector b) noexcept
{
    return transform(a, b);
}


constexpr matrix scale(vector scale) noexcept
{
    if (std::is_constant_evaluated()) {

        return {
            { scale[0], 0.0f, 0.0f, 0.0f },
            { 0.0f, scale[1], 0.0f, 0.0f },
            { 0.0f, 0.0f, scale[2], 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
    } else {
        return {
            scale & vector(identity_mask[0]),
            scale & vector(identity_mask[1]),
            scale & vector(identity_mask[2]),
            identity[3]
        };
    }
}
constexpr matrix translate(vector translation) noexcept
{
    if (std::is_constant_evaluated()) {
        return {
            identity[0],
            identity[1],
            identity[2],
            { translation[0], translation[1], translation[2], 1.0f }
        };
    } else {
        return {
            identity[0],
            identity[1],
            identity[2],
            _mm_or_ps(_mm_andnot_ps(vector(identity_mask[3]), translation), identity[3])
        };
    }
}

constexpr matrix transpose(const matrix& a) noexcept
{
    if (std::is_constant_evaluated()) {
        return {
            { a[0][0], a[1][0], a[2][0], a[3][0] },
            { a[0][1], a[1][1], a[2][1], a[3][1] },
            { a[0][2], a[1][2], a[2][2], a[3][2] },
            { a[0][3], a[1][3], a[2][3], a[3][3] }
        };
    } else {
        __m256 t0 = _mm256_castps128_ps256(a[0]);
        t0 = _mm256_insertf128_ps(t0, a[1], 1);
        __m256 t1 = _mm256_castps128_ps256(a[2]);
        t1 = _mm256_insertf128_ps(t1, a[3], 1);

        __m256 vTemp = _mm256_unpacklo_ps(t0, t1);
        __m256 vTemp2 = _mm256_unpackhi_ps(t0, t1);
        __m256 vTemp3 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x20);
        __m256 vTemp4 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x31);
        vTemp = _mm256_unpacklo_ps(vTemp3, vTemp4);
        vTemp2 = _mm256_unpackhi_ps(vTemp3, vTemp4);
        t0 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x20);
        t1 = _mm256_permute2f128_ps(vTemp, vTemp2, 0x31);

        return {
            _mm256_castps256_ps128(t0),
            _mm256_extractf128_ps(t0, 1),
            _mm256_castps256_ps128(t1),
            _mm256_extractf128_ps(t1, 1)
        };
    }
}
} // namespace w::math