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

# SDL2
CPMAddPackage(
  NAME SDL3
  GITHUB_REPOSITORY libsdl-org/SDL
  GIT_TAG preview-3.1.3
  OPTIONS
  "SDL_WERROR OFF"
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
