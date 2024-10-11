#pragma once
#include <wisdom/wisdom.hpp>

#ifndef NDEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x)
#endif

namespace w {
inline constexpr bool failed(wis::Result res) noexcept
{
    return int(res.status) < 0;
}
} // namespace w