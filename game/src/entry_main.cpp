#include <base/await.h>
#include <iostream>
#include <base/tasks.h>

w::task<void> async_task2()
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Hello from async_task2!" << std::endl;

    co_return;
}

w::fire_and_forget async_task()
{
    co_await async_task2();

    std::cout << "Hello from async_task!" << std::endl;
    throw 1;
}

int main()
{
    auto scoped_pool = w::base::thread_pool::init_scoped();
    try
    {
        async_task();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    catch (int e)
    {
        std::cout << "Caught exception: " << e << std::endl;
    }
    std::cout << "Hello from main!" << std::endl;
    return 0;
}