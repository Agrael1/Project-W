#pragma once
#include <math/matrix.h>

namespace w::math {
constexpr matrix multiply(const matrix& a, const matrix& b) noexcept
{
    if (std::is_constant_evaluated())
    {

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