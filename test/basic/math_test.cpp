#include <catch2/catch_test_macros.hpp>
#include <math/vector_math.h>

using namespace w::math;

static_assert(sizeof(w::math::vector) == 16, "vector size must be 16 bytes");
static_assert(alignof(w::math::vector) == 16, "vector alignment must be 16 bytes");

void test_vector_addition_constexpr()
{
    constexpr vector a(1.0f, 2.0f, 3.0f, 4.0f);
    constexpr vector b(5.0f, 6.0f, 7.0f, 8.0f);
    constexpr vector c = a + b;
    static_assert(c[0] == 6.0f);
    static_assert(c[1] == 8.0f);
    static_assert(c[2] == 10.0f);
    static_assert(c[3] == 12.0f);
}

TEST_CASE("vector_add")
{
    vector a(1.0f, 2.0f, 3.0f, 4.0f);
    vector b(5.0f, 6.0f, 7.0f, 8.0f);
    vector c = a + b;
    REQUIRE(c[0] == 6.0f);
    REQUIRE(c[1] == 8.0f);
    REQUIRE(c[2] == 10.0f);
    REQUIRE(c[3] == 12.0f);

    float4 d(1.0f, 2.0f, 3.0f, 4.0f);
    float4 e(5.0f, 6.0f, 7.0f, 8.0f);
    float4 f = d + e;
    REQUIRE(f[0] == 6.0f);
    REQUIRE(f[1] == 8.0f);
    REQUIRE(f[2] == 10.0f);
    REQUIRE(f[3] == 12.0f);
}

void test_matrix_mul_constexpr()
{
    
}