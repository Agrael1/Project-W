#pragma once
#include <math/matrix.h>

namespace w::math {
constexpr matrix multiply(const matrix& a, const matrix& b) noexcept
{
    if (std::is_constant_evaluated()) {
        matrix result;
        // Cache the invariants in registers
        float x = a[0][0];
        float y = a[0][1];
        float z = a[0][2];
        float w = a[0][3];
        // Perform the operation on the first row

        result[0] = {
            (b[0][0] * x) + (b[1][0] * y) + (b[2][0] * z) + (b[3][0] * w),
            (b[0][1] * x) + (b[1][1] * y) + (b[2][1] * z) + (b[3][1] * w),
            (b[0][2] * x) + (b[1][2] * y) + (b[2][2] * z) + (b[3][2] * w),
            (b[0][3] * x) + (b[1][3] * y) + (b[2][3] * z) + (b[3][3] * w),
        };

        // Repeat for all the other rows
        x = a[1][0];
        y = a[1][1];
        z = a[1][2];
        w = a[1][3];
        result[1] = {
            (b[0][0] * x) + (b[1][0] * y) + (b[2][0] * z) + (b[3][0] * w),
            (b[0][1] * x) + (b[1][1] * y) + (b[2][1] * z) + (b[3][1] * w),
            (b[0][2] * x) + (b[1][2] * y) + (b[2][2] * z) + (b[3][2] * w),
            (b[0][3] * x) + (b[1][3] * y) + (b[2][3] * z) + (b[3][3] * w),
        };

        x = a[2][0];
        y = a[2][1];
        z = a[2][2];
        w = a[2][3];
        result[2] = {
            (b[0][0] * x) + (b[1][0] * y) + (b[2][0] * z) + (b[3][0] * w),
            (b[0][1] * x) + (b[1][1] * y) + (b[2][1] * z) + (b[3][1] * w),
            (b[0][2] * x) + (b[1][2] * y) + (b[2][2] * z) + (b[3][2] * w),
            (b[0][3] * x) + (b[1][3] * y) + (b[2][3] * z) + (b[3][3] * w),
        };

        x = a[3][0];
        y = a[3][1];
        z = a[3][2];
        w = a[3][3];
        result[3] = {
            (b[0][0] * x) + (b[1][0] * y) + (b[2][0] * z) + (b[3][0] * w),
            (b[0][1] * x) + (b[1][1] * y) + (b[2][1] * z) + (b[3][1] * w),
            (b[0][2] * x) + (b[1][2] * y) + (b[2][2] * z) + (b[3][2] * w),
            (b[0][3] * x) + (b[1][3] * y) + (b[2][3] * z) + (b[3][3] * w),
        };
        return result;
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

} // namespace w::math