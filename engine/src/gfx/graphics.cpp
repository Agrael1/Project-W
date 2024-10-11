#include <gfx/graphics.h>
#include <gfx/platform.h>
#include <gfx/misc.h>
#include <wisdom/wisdom_debug.h>
#include <format>

// TODO: Elaborate
struct LogProvider : public wis::LogLayer {
    virtual void Log(wis::Severity sev, std::string message, wis::source_location sl = wis::source_location::current()) override
    {
        std::cout << std::format("[{}]: {}\n", wis::severity_strings[+sev], message);
    };
};

void DebugCallback(wis::Severity severity, const char* message, void* user_data)
{
    auto stream = reinterpret_cast<std::ostream*>(user_data);
    *stream << message << "\n";
}

w::action<void> w::graphics::init_async(platform_extension& ext)
{
    co_await w::resume_background();
    wis::LibLogger::SetLogLayer(std::make_shared<LogProvider>());

    // Create the factory
    wis::DebugExtension debug_ext;
    wis::FactoryExtension* factory_exts[] = { ext.get(), &debug_ext };
    auto [r1, factory] = wis::CreateFactory(true, factory_exts, std::size(factory_exts));
    if (w::failed(r1)) {
        // log error
        co_return;
    }

#ifndef NDEBUG
    // Create the debug messenger
    auto [r2, hdebug] = debug_ext.CreateDebugMessenger(DebugCallback, &std::cout);
    if (w::failed(r2)) {
        // log error
        co_return;
    }
    debug_info = std::move(hdebug);
#endif // !NDEBUG

    // Create the device
    create_device(factory);

    // Create the main queue
    auto [r3, queue] = device.CreateCommandQueue(wis::QueueType::Graphics);
    if (w::failed(r3)) {
        // log error
        co_return;
    }
    main_queue = std::move(queue);

    // Create the allocator
    auto [r4, alloc] = device.CreateAllocator();
    if (w::failed(r4)) {
        // log error
        co_return;
    }
    allocator = std::move(alloc);
    co_return;
}

void w::graphics::create_device(const wis::Factory& factory)
{
    for (size_t i = 0;; i++) {
        auto [res, adapter] = factory.GetAdapter(i);
        if (res.status == wis::Status::Ok) {
            wis::AdapterDesc desc;
            res = adapter.GetDesc(&desc);

            // TODO: Proper logging
            std::cout << wis::format("Graphics adapter: {}\n", desc.description.data());

            wis::DeviceExtension* device_exts[] = { &extended_alloc };
            auto [res, hdevice] = wis::CreateDevice(std::move(adapter), device_exts, std::size(device_exts));
            if (res.status == wis::Status::Ok) {
                device = std::move(hdevice);
                break;
            } else {
                // Log warning
            }
        }
    }
}
