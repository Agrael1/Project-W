project(Game)

set(SOURCES "src/entry_main.cpp" "src/app.cpp" "src/window.cpp")
set(HEADERS "include/app.h" "include/window.h")

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE WEngine)
WIS_INSTALL_DEPS(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# Copy the dlls to the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	$<TARGET_FILE:SDL3::SDL3>
	$<TARGET_FILE_DIR:${PROJECT_NAME}>
)