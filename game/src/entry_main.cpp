#include <base/await.h>
#include <iostream>
#include <base/tasks.h>
#include <base/thread_pool.h>

#include <app.h>
#include <span>
#include <ranges>



static w::action<int> main_async(int argc, char** argv)
{
    std::span<char*> args(argv + 1, argc - 1);

    bool fullscreen = false;
    for (auto arg : args | std::views::transform([](auto arg) { return std::string_view(arg); })) {
        if (arg == "--fullscreen") {
            fullscreen = true;
        }
    }

    // Create an app
    ut::app app;
    co_await app.init_async();
    co_return 0;
}

// Staged to reach thread pool
static w::action<int> main_stage_async(int argc, char** argv)
{
    co_await w::resume_background();
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