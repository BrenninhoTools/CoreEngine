set(COREENGINE_SDL_TAG "release-3.2.10" CACHE STRING "SDL3 git tag used when SDL3 is fetched")
set(COREENGINE_SDL_DIR "${CMAKE_SOURCE_DIR}/third_party/SDL" CACHE PATH "Local SDL3 source directory")

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "" FORCE)

if(ANDROID)
  set(SDL_SHARED ON CACHE BOOL "" FORCE)
  set(SDL_STATIC OFF CACHE BOOL "" FORCE)
else()
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
endif()

if(EXISTS "${COREENGINE_SDL_DIR}/CMakeLists.txt")
  add_subdirectory("${COREENGINE_SDL_DIR}" "${CMAKE_BINARY_DIR}/sdl")
else()
  include(FetchContent)
  FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG "${COREENGINE_SDL_TAG}"
    GIT_SHALLOW ON
  )
  FetchContent_MakeAvailable(SDL3)
endif()
