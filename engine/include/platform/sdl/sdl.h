#pragma once
#include <platform/sdl/window.h>
#include <base/result.h>

namespace w::sdl {
class sdl_factory {
    friend w::result<sdl_factory> create_factory() noexcept;

public:
    sdl_factory() noexcept = default;
    sdl_factory(const sdl_factory&) = delete;
    sdl_factory& operator=(const sdl_factory&) = delete;
    sdl_factory(sdl_factory&& o) noexcept
        : initialized(std::exchange(o.initialized, false)) { }
    sdl_factory& operator=(sdl_factory&& o) noexcept
    {
        if (this != &o) {
            initialized = std::exchange(o.initialized, false);
        }
    }
    ~sdl_factory() noexcept;

public:
    w::result<window>
    create_window(const char* title, int w, int h, uint32_t flags) const;

private:
    sdl_factory(bool) noexcept { initialized = true; }
    bool initialized = false;
};

w::result<sdl_factory> create_factory() noexcept;
} // namespace w::sdl