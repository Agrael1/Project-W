project(WEngine)

set(HEADERS "include/graphics.h" "include/swapchain.h" "include/base/thread_pool.h" "include/base/await.h")
set(SOURCES "src/graphics.cpp" "src/base/thread_pool.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC wis::debug wis::platform wis::extended-allocation)
target_link_libraries(${PROJECT_NAME} PUBLIC winrt dp::thread-pool)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)