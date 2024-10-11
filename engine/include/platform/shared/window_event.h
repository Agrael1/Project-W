#pragma once
#include <array>
#include <bitset>

namespace w {
enum class window_event {
    NoEvent = 0,
    Quit,
    Resize,
    Restyle,
    LoadAsset,
    Play,
    Count
};
}
