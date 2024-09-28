include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(GET_CPM_FILE "${CMAKE_CURRENT_LIST_DIR}/get_cpm.cmake")

if (NOT EXISTS ${GET_CPM_FILE})
  file(DOWNLOAD
      https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
      "${GET_CPM_FILE}"
  )
endif()
include(${GET_CPM_FILE})

# Add CPM dependencies here
CPMAddPackage(
  NAME Wisdom
  GITHUB_REPOSITORY Agrael1/Wisdom
  GIT_TAG master
)

CPMAddPackage(
  NAME Xenium
  GITHUB_REPOSITORY mpoeter/xenium
  GIT_TAG master
)


if(PROJECTW_TESTS)
  CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    GIT_TAG v3.4.0
    EXCLUDE_FROM_ALL
  )
  list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
  include(Catch)
endif()

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


