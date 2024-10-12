#include <gfx/platform.h>
#include <gfx/graphics.h>
#include <platform/sdl/window.h>
#include <SDL3/SDL_video.h>

w::platform_extension::platform_extension(const w::sdl::sdl_factory&)
{
    current = selector::None;
    platform = {};
#if defined(SDL_PLATFORM_WIN32)
    platform = std::make_unique<wis::platform::WindowsExtension>();
    current = selector::Windows;
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
        platform = std::make_unique<wis::platform::XCBExtension>();
        current = selector::XCB;
    } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        platform = std::make_unique<wis::platform::WaylandExtension>();
        current = selector::Wayland;
    }
#endif
}

w::result<w::swapchain> w::platform_extension::create_swapchain(w::graphics& gfx, const w::sdl::window& wnd) const noexcept
{
    SDL_Window* window = wnd.get();
    if (!window) {
        return { "No window is passed", w::error };
    }

    if (current == selector::None) {
        return { "Platform is not selected", w::error };
    }

    auto [r, fence] = gfx.create_fence();
    if (failed(r)) {
        return { r.error, w::error };
    }

    auto [width, height] = wnd.pixel_size();
    wis::SwapchainDesc desc{
        .size = { uint32_t(width), uint32_t(height) },
        .format = wis::DataFormat::BGRA8Unorm,
        .buffer_count = 2,
        .stereo = false,
        .vsync = true,
    };

    switch (current) {
#if defined(SDL_PLATFORM_WIN32)
    case selector::Windows: {
        HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        if (hwnd) {
            auto [r, swap] = static_cast<wis::platform::WindowsExtension*>(platform.get())->CreateSwapchain(gfx.get_device(), gfx.get_main_queue(), &desc, hwnd);
            if (r.status != wis::Status::Ok) {
                return { r.error, w::error };
            }
            return w::swapchain{ std::move(swap), std::move(fence) };
        }
    } break;
#elif defined(SDL_PLATFORM_LINUX)
    case selector::XCB: {
        Display* xdisplay = (Display*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        Window xwindow = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (xdisplay && xwindow) {
            assert(false && "X11 platform is unavailable");
        }
    } break;
    case selector::Wayland: {
        struct wl_display* display = (struct wl_display*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        struct wl_surface* surface = (struct wl_surface*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        if (display && surface) {
            auto [r, swap] = static_cast<wis::platform::WaylandExtension*>(platform.get())
                                     ->CreateSwapchain(gfx.get_device(), gfx.get_main_queue(), &desc, display, surface);
            if (r.status != wis::Status::Ok) {
                return { r.error, w::error };
            }
            return w::swapchain{ std::move(swap), std::move(fence) };
        }
    } break;
#endif
    }
    return { "Platform is not selected", w::error };
}
