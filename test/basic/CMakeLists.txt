project("test-basic")

set(TEST_SOURCES "coro_test.cpp" "async_queue.cpp")

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC Catch2::Catch2WithMain WEngine)
catch_discover_tests(${PROJECT_NAME} TEST_PREFIX "prw.${PROJECT_NAME}.")