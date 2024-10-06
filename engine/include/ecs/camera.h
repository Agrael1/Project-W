#pragma once
#include <ecs/transform.h>

namespace w::ecs {
struct camera : public transform {
    mutable math::float4x4a view;

public:
    math::matrix get_view() const noexcept
    {
        if (dirty()) {
            constexpr static math::vector forward = math::identity[2];
            auto look_vector = math::transform(get_rotation(), forward);
            view = math::look_to(get_position(), look_vector, math::identity[1]);
        }
        return view;
    }
};

}