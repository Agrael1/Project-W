find_package(wisdom QUIET)
if(NOT wisdom_FOUND)
  FetchContent_Declare(
    wisdom
    GIT_REPOSITORY https://github.com/Agrael1/Wisdom
    GIT_TAG master)
    FetchContent_GetProperties(wisdom)
    if(NOT wisdom_POPULATED)
	  FetchContent_Populate(wisdom)
	  add_subdirectory(${wisdom_SOURCE_DIR} ${wisdom_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
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
