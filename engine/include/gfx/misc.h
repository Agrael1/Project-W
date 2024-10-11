#pragma once
#include <wisdom/wisdom.hpp>
#include <base/result.h>

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
inline constexpr w::error_message to_error(wis::Result res) noexcept
{
    return failed(res) ? w::error_message{ res.error } : w::error_message{};
}
} // namespace w