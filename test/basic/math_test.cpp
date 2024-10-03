#include <catch2/catch_test_macros.hpp>
#include <math/matrix_math.h>
#include <math/quaternion_math.h>

using namespace w::math;

static_assert(sizeof(w::math::vector) == 16, "vector size must be 16 bytes");
static_assert(alignof(w::math::vector) == 16, "vector alignment must be 16 bytes");

void test_vector_constexpr()
{
    constexpr vector a(1.0f, 2.0f, 3.0f, 4.0f);
    constexpr vector b(5.0f, 6.0f, 7.0f, 8.0f);
    constexpr vector c = a + b;
    static_assert(c[0] == 6.0f);
    static_assert(c[1] == 8.0f);
    static_assert(c[2] == 10.0f);
    static_assert(c[3] == 12.0f);

    constexpr vector d = opposite(a);
    static_assert(equal(d, vector(-1.0f, -2.0f, -3.0f, 4.0f)));
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
    constexpr auto scale_vector = vector(2.0f, 3.0f, 4.0f, 1.0f);
    constexpr auto scale_vector2 = vector(2.0f, 0.0f, 1.0f, 1.0f);
    constexpr auto m = scale(scale_vector);
    constexpr auto m2 = scale(scale_vector2);

    constexpr auto m3 = m * m2;
    static_assert(m3[0][0] == 4.0f);

    constexpr auto m4 = translate(scale_vector);

    constexpr auto m5 = transform(m, scale_vector2);
}

TEST_CASE("matrix_mul")
{
    constexpr auto scale_vector = vector(2.0f, 3.0f, 4.0f, 1.0f);
    constexpr auto scale_vector2 = vector(2.0f, 0.0f, 1.0f, 1.0f);
    constexpr auto m = scale(scale_vector);
    constexpr auto m2 = scale(scale_vector2);

    constexpr auto m3 = m * m2; //constexpr version

    auto scale_vector3 = vector(2.0f, 3.0f, 4.0f, 1.0f);
    auto scale_vector4 = vector(2.0f, 0.0f, 1.0f, 1.0f);
    auto m4 = scale(scale_vector3);
    auto m5 = scale(scale_vector4);

    auto m6 = m4 * m5; // non-constexpr version

    REQUIRE(m3[0][0] == 4.0f);
    REQUIRE(m6[0][0] == 4.0f);

    REQUIRE(equal(m3[0], m6[0]));
    REQUIRE(equal(m3[1], m6[1]));
    REQUIRE(equal(m3[2], m6[2]));
    REQUIRE(equal(m3[3], m6[3]));

    // test transform

    constexpr auto v8 = transform(m, scale_vector2);
    auto v7 = transform(m4, scale_vector4);

    REQUIRE(equal(v7, v8));
}