include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(GET_CPM_FILE "${CMAKE_CURRENT_LIST_DIR}/get_cpm.cmake")


file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
    "${GET_CPM_FILE}"
)
include(${GET_CPM_FILE})

# Add CPM dependencies here
CPMAddPackage(
  NAME Wisdom
  GITHUB_REPOSITORY Agrael1/Wisdom
  GIT_TAG master
)


find_package(thread-pool QUIET)
if(NOT thread-pool_FOUND)
  FetchContent_Declare(
    thread_pool
    GIT_REPOSITORY https://github.com/DeveloperPaul123/thread-pool.git
    GIT_TAG master)
    FetchContent_GetProperties(thread_pool)
    if(NOT thread_pool_POPULATED)
	  FetchContent_Populate(thread_pool)
	  add_subdirectory(${thread_pool_SOURCE_DIR} ${thread_pool_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()


