#pragma once
#include <math/vector_math.h>
#include <math/matrix_math.h>
#include <math/quaternion_math.h>

namespace w::ecs {
struct transform {
private:
    math::float3a position;
    math::quat<math::float4a> rotation;
    math::float3a scale;

    // Cached matrix
    mutable math::float4x4a matrix;
    mutable bool _dirty = true; // Whether the matrix needs to be recalculated
    transform* parent = nullptr; // Parent transform

public:
    bool dirty() const noexcept
    {
        return _dirty || (parent && parent->dirty());
    }
    void set_dirty() noexcept
    {
        _dirty = true;
    }
    void set_position(math::vector pos) noexcept
    {
        position = pos;
        _dirty = true;
    }
    void set_rotation(math::quaternion rot) noexcept
    {
        rotation = rot;
        _dirty = true;
    }
    void set_scale(math::vector s) noexcept
    {
        scale = s;
        _dirty = true;
    }
    void set_parent(transform* p) noexcept
    {
        parent = p;
        _dirty = true;
    }

public:
    math::vector get_position() const noexcept
    {
        return position;
    }
    math::quaternion get_rotation() const noexcept
    {
        return math::quaternion(rotation);
    }
    math::vector get_scale() const noexcept
    {
        return scale;
    }
    transform* get_parent() const noexcept
    {
        return parent;
    }

    math::matrix local_matrix() const noexcept
    {
        auto qrotation = math::matrix(math::quaternion(rotation));
        return math::scale(scale) * qrotation * math::translate(position);
    }
    math::matrix world_matrix() const noexcept
    {
        if (dirty()) {
            matrix = parent ? parent->world_matrix() * local_matrix() : local_matrix();
            _dirty = false;
        }
        return matrix;
    }
};
} // namespace w::esc