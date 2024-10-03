#pragma once
#include <math/vector_math.h>

namespace w::math {
struct quaternion : vector {
    using vector::vector;
};

struct angle_axis : vector {
    constexpr angle_axis() = default;
    constexpr angle_axis(float angle, vector axis) noexcept
        : vector(init(angle, axis))
    {
    }

public:
    static constexpr vector init(float angle, vector axis) noexcept
    {
        if (std::is_constant_evaluated()) {
            return {
                axis[0],
                axis[1],
                axis[2],
                angle
            };
        } else {
            constexpr vector select = vector(identity_mask[3]);

            vector tmp = axis & select;
            vector xangle = _mm_andnot_ps(select, vector(angle, broadcast));
            return tmp | xangle;
        }
    }
    operator quaternion()const noexcept;
};


} // namespace w::math