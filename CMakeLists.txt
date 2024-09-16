# CMakeList.txt : CMake project for WEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.22)
project ("WEngine")

option(PROJECTW_TESTS "Build the tests" ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# CPM package manager
include(cmake/deps.cmake)

set(CPM_SOURCE_CACHE "${CMAKE_BINARY_DIR}/_deps" CACHE STRING "Where to cache CPM dependencies")

add_subdirectory("engine")
add_subdirectory("game")

if(PROJECTW_TESTS)
  enable_testing()
  add_subdirectory("test")
endif()