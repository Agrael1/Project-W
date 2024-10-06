#include <base/await.h>
#include <iostream>
#include <base/tasks.h>
#include <base/thread_pool.h>

static w::action<int> async_main()
{
    std::cout << "Hello from async_main!" << std::endl;
    co_return 42;
}

int main()
{
    auto token = w::base::global_thread_pool_token::init_scoped();
    try {
        return async_main().get();
    } catch (int e) {
        std::cout << "Caught exception: " << e << std::endl;
        return -1;
    }
}