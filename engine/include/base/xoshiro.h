/*  Written in 2016 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */
#pragma once
#include <stdint.h>

/* This is xoroshiro64* 1.0, our best and fastest 32-bit small-state
   generator for 32-bit floating-point numbers. We suggest to use its
   upper bits for floating-point generation, as it is slightly faster than
   xoroshiro64**. It passes all tests we are aware of except for linearity
   tests, as the lowest six bits have low linear complexity, so if low
   linear complexity is not considered an issue (as it is usually the
   case) it can be used to generate 32-bit outputs, too.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   The state must be seeded so that it is not everywhere zero. */

struct xoroshiro {
    constexpr xoroshiro(uint32_t seed) noexcept
    {
        s[0] = seed;
        s[1] = 0x9E3779BB;
    }
    constexpr xoroshiro(uint32_t s0, uint32_t s1) noexcept
    {
        s[0] = s0;
        s[1] = s1;
    }
    xoroshiro(const xoroshiro&) = delete;
    xoroshiro& operator=(const xoroshiro&) = delete;
    constexpr xoroshiro(xoroshiro&&) noexcept = default;
    constexpr xoroshiro& operator=(xoroshiro&&) noexcept = default;

public:
    static constexpr inline uint32_t rotl(const uint32_t x, int k) noexcept
    {
        return (x << k) | (x >> (32 - k));
    }
    constexpr uint32_t next() noexcept
    {
        const uint32_t s0 = s[0];
        uint32_t s1 = s[1];
        const uint32_t result = s0 * 0x9E3779BB;

        s1 ^= s0;
        s[0] = rotl(s0, 26) ^ s1 ^ (s1 << 9); // a, b
        s[1] = rotl(s1, 13); // c

        return result;
    }

public:
    uint32_t s[2]; // state
};
