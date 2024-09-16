#include <base/await.h>
#include <iostream>
#include <base/tasks.h>

w::task<int> async_task3()
{
    co_await w::resume_background();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Hello from async_task3!" << std::endl;

    co_return 2;
}

w::task<void> async_task2()
{

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Hello from async_task2!" << std::endl;

    co_return;
}

w::action<void> async_action()
{
    co_await w::resume_background();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //throw 1;

    std::cout << "Hello from async_action!" << std::endl;
    co_return;
}

w::fire_and_forget async_task()
{
    //co_await async_task2();

    auto a = async_action();
    //throw 1;

    co_await a;
    std::cout << "Hello from async_task!" << std::endl;
}

int main()
{
    auto scoped_pool = w::base::thread_pool::init_scoped();
    try
    {
        async_task();
    }
    catch (int e)
    {
        std::cout << "Caught exception: " << e << std::endl;
    }
    std::cout << "Hello from main!" << std::endl;
    return 0;
}