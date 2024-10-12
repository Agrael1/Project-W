project(WEngine)

set(HEADERS 
"include/gfx/graphics.h" 
"include/gfx/swapchain.h" 
"include/gfx/platform.h" 
"include/gfx/misc.h" 
"include/base/thread_pool.h" 
"include/base/xoshiro.h" 
"include/base/await.h" 
"include/base/await_traits.h" 
"include/base/atomic_queue.h" 
"include/base/tasks.h"  
"include/base/atomic_buffer.h"  
"include/base/event_count.h"  
"include/math/vector.h"  
"include/math/vector_math.h"  
"include/math/matrix.h"  
"include/math/matrix_math.h"  
"include/math/quaternion.h"  
"include/math/quaternion_math.h"
"include/ecs/transform.h"
"include/ecs/camera.h"
"include/platform/sdl/sdl.h"
"include/platform/sdl/window.h"
"include/platform/shared/window_event.h"
  
  
 "include/base/stealing_deque.h" "include/base/result.h")
set(SOURCES "src/gfx/graphics.cpp" "src/base/thread_pool.cpp" "src/sdl/window.cpp" "src/sdl/sdl.cpp" "src/gfx/platform.cpp" "src/gfx/swapchain.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC wis::debug wis::platform wis::extended-allocation)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)