#include <base/await.h>
#include <iostream>

w::fire_and_forget main_async()
{
    co_await w::resume_background();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Hello, world!" << std::endl;
}

int main()
{
    auto scoped_pool = w::base::thread_pool::init_scoped();

    main_async();
    std::cout << "Hello from main!" << std::endl;
    return 0;
}