#include <catch2/catch_test_macros.hpp>
#include <base/async_queue.h>
#include <thread>
#include <vector>
#include <set>
#include <mutex>

class Test
{
public:
    Test(std::set<int>& storage, uint32_t thread_count = std::thread::hardware_concurrency() - 1)
    {
        threads.reserve(thread_count);
        for (size_t i = 0; i < thread_count; ++i) {
            threads.emplace_back([this, &storage] {
                _start.wait(false, std::memory_order::acquire);
                while (true) {
                    auto item = queue.try_steal();
                    if (item) {
                        // do something with item
                        printf("Thread %d: %d\n", std::this_thread::get_id(), item.value());
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        if (item.value() != 0) {
                            std::unique_lock lock(mutex);
                            REQUIRE(storage.find(item.value()) == storage.end());
                            storage.insert(item.value());
                            overflow.store(false, std::memory_order_release);
                            overflow.notify_one();
                        } 
                    } else {
                        if (_stop.load(std::memory_order_relaxed)) {
                            break;
                        }

                        has_something.store(false, std::memory_order_release);
                        printf("Thread %d: waiting...\n", std::this_thread::get_id());
                        has_something.wait(false, std::memory_order::acquire);
                        printf("Thread %d: done waiting\n", std::this_thread::get_id());
                    }
                }
            });
        }
    }

public:
    void push(int item)
    {
        while (true) {
            if (queue.try_push(item)) {
                has_something.store(true, std::memory_order_release);
                printf("Pushed %d, notifying\n", item);
                has_something.notify_one();
                return;
            } else {
                printf("Failed to push %d\n", item);
                overflow.store(true, std::memory_order_release);
                overflow.wait(true, std::memory_order::acquire);
            }
        }
    }
    void pop(std::set<int>& storage)
    {
        auto item = queue.try_pop();
        if (item) {
            std::unique_lock lock(mutex);
            // do something with item
            printf("Popped %d\n", item.value());

            if (item.value() != 0) {
                REQUIRE(storage.find(item.value()) == storage.end());
                storage.insert(item.value());
            }
            overflow.store(false, std::memory_order_release);
            overflow.notify_one();
        }
    }
    void start()
    {
        _start.store(true, std::memory_order_relaxed);
        _start.notify_all();
    }
    void stop()
    {
        has_something.store(true, std::memory_order_release);
        has_something.notify_all();
        _stop.store(true, std::memory_order_relaxed);
        for (auto& thread : threads) {
            thread.join();
        }
    }

public:
    w::base::stealing_deque<int, 128> queue;
    std::vector<std::jthread> threads;
    std::atomic<bool> _stop{ false };
    std::atomic<bool> _start{ false };
    std::atomic<bool> has_something{ false };
    std::atomic<bool> overflow{ false };
    std::mutex mutex;
};

TEST_CASE("basic_stealing")
{
    std::set<int> storage;
    std::set<int> storage2;
    Test test(storage);
    for (int i = 1; i < 11; ++i) {
        storage2.insert(i);
        test.push(i);
    }
    test.start();
    test.stop();
    REQUIRE(storage == storage2);
}

TEST_CASE("advanced_stealing")
{
    std::set<int> storage;
    std::set<int> storage2;
    Test test(storage);
    for (int i = 1; i < 11; ++i) {
        storage2.insert(i);
        test.push(i);
    }
    test.start();
    for (int i = 11; i < 21; ++i) {
        storage2.insert(i);
        test.push(i);
    }
    test.stop();
    REQUIRE(storage == storage2);
}

TEST_CASE("concurrent_madness")
{
    static int counter = 1;
    std::set<int> storage;
    std::set<int> storage2;
    Test test(storage);
    test.start();

    for (size_t i = 1; i < 11; ++i) {
        test.push(counter);
        storage2.insert(counter++);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    for (size_t i = 0; i < 5; i++) {
        test.pop(storage);
        if (rand() % 2) {
            storage2.insert(counter);
            test.push(counter++);
        }
    }

    for (size_t i = 1; i < 11; ++i) {
        test.push(counter);
        storage2.insert(counter++);
    }
    test.stop();
    REQUIRE(storage == storage2);
}

TEST_CASE("smoke_madness")
{
    static int counter = 1;
    std::set<int> storage;
    std::set<int> storage2;
    Test test(storage);
    test.start();
    

    for (size_t r = 1; r < 110; ++r)
    {
        for (size_t i = 1; i < 110; ++i) {
            test.push(counter);
            storage2.insert(counter++);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        for (size_t i = 0; i < 50; i++) {
            test.pop(storage);
            if (rand() % 2) {
                storage2.insert(counter);
                test.push(counter++);
            }
        }

        for (size_t i = 1; i < 110; ++i) {
            test.push(counter);
            storage2.insert(counter++);
        }
    }
    test.stop();

    REQUIRE(storage.size() == storage2.size());
    REQUIRE(storage == storage2);
}