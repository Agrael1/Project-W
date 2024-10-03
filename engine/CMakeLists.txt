project(WEngine)

set(HEADERS 
"include/graphics.h" 
"include/swapchain.h" 
"include/base/thread_pool.h" 
"include/base/await.h" 
"include/base/await_traits.h" 
"include/base/tasks.h"  
"include/base/atomic_buffer.h"  
"include/math/vector.h"  
"include/math/vector_math.h"  
"include/math/matrix.h"  
"include/math/matrix_math.h"  
"include/math/quaternion.h"  
"include/math/quaternion_math.h"
  
  
 "include/base/async_queue.h")
set(SOURCES "src/graphics.cpp" "src/base/thread_pool.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PUBLIC wis::debug wis::platform wis::extended-allocation

)
target_link_libraries(${PROJECT_NAME} PUBLIC dp::thread-pool)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)