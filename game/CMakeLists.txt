project(Game)

set(SOURCES "src/entry_main.cpp")
set(HEADERS)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE WEngine)
WIS_INSTALL_DEPS(${PROJECT_NAME})