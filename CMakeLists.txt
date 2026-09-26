cmake_minimum_required(VERSION 3.22)

project(CoreEngine VERSION 0.1.0 LANGUAGES C CXX)

if(WIN32)
  enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(ANDROID OR IOS)
  set(COREENGINE_EDITOR_DEFAULT OFF)
else()
  set(COREENGINE_EDITOR_DEFAULT ON)
endif()

option(COREENGINE_BUILD_SANDBOX "Build the sandbox application" ON)
option(COREENGINE_BUILD_EDITOR "Build the editor and home screen" ${COREENGINE_EDITOR_DEFAULT})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CoreEngineSDL)
include(CoreEngineApp)

add_subdirectory(engine)

if(COREENGINE_BUILD_SANDBOX)
  add_subdirectory(sandbox)
endif()

if(COREENGINE_BUILD_EDITOR)
  include(CoreEngineImGui)
  add_subdirectory(editor)
endif()
