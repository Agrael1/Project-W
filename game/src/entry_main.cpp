#include <base/await.h>
#include <iostream>
#include <base/tasks.h>
#include <base/thread_pool.h>

#include <app.h>
#include <span>
#include <ranges>
#include <codecvt>


static w::action<int> main_async(int argc, char** argv)
{
    auto ui_thread = w::global::current();
    std::span<char*> args(argv + 1, argc - 1);

    uint32_t width = 800;
    uint32_t height = 600;
    bool fullscreen = false;
    for (auto arg : args | std::views::transform([](auto arg) { return std::string_view(arg); })) {
        if (arg == "--fullscreen") {
            fullscreen = true;
        } else if (arg.starts_with("--width=")) {
            if (std::from_chars(arg.data() + 8, arg.data() + arg.size(), width).ec == std::errc::invalid_argument) {
                std::cerr << "Invalid width argument: " << arg.substr(8) << std::endl;
                co_return -1;
            }
        } else if (arg.starts_with("--height=")) {
            if (std::from_chars(arg.data() + 9, arg.data() + arg.size(), height).ec == std::errc::invalid_argument) {
                std::cerr << "Invalid height argument: " << arg.substr(9) << std::endl;
                co_return -1;
            }
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            co_return -1;
        }
    }

    // Create an app
    ut::app app{ ui_thread };
    co_await app.init_async(width, height, fullscreen);
    co_return 0;
}

// Staged to reach thread pool
static w::action<int> main_stage_async(int argc, char** argv)
{
    // Resume on the background thread pool, here always on 0 thread
    auto ui_thread = w::global::current();
    co_await w::resume_affine(ui_thread);
    co_return co_await main_async(argc, argv);
}


int main(int argc, char** argv)
{
    auto token = w::base::global_thread_pool_token::init_scoped();
    try {
        return main_stage_async(argc, argv).get();
    } catch (int e) {
        std::cout << "Caught exception: " << e << std::endl;
        return -1;
    }
}