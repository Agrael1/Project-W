#pragma once
#include <base/result.h>

namespace w::sdl {
struct scoped_sdl {
    friend w::result<scoped_sdl> init_sdl() noexcept;
    ~scoped_sdl() noexcept;
    scoped_sdl() noexcept = default;

private:
    bool initialized = false;
};

w::result<scoped_sdl> init_sdl() noexcept;
void uninit_sdl() noexcept;
} // namespace w::sdl