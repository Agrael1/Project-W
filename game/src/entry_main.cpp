#include <base/await.h>
#include <iostream>
#include <base/tasks.h>
#include <base/thread_pool.h>


w::action<int> async_main()
{
    std::cout << "Hello from async_main!" << std::endl;
    co_return 42;
}

int main()
{
    auto token = w::base::global_thread_pool_token::init_scoped();
    try
    {
        async_main();
    }
    catch (int e)
    {
        std::cout << "Caught exception: " << e << std::endl;
    }
    std::cout << "Hello from main!" << std::endl;
    return 0;
}