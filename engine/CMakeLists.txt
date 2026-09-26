add_library(coreengine STATIC
  src/Application.cpp
  src/Icon.cpp
  src/Input.cpp
  src/Platform.cpp
  src/Renderer.cpp
  src/Scene.cpp
  src/SceneData.cpp
  src/Window.cpp
)

add_library(CoreEngine::Engine ALIAS coreengine)

target_include_directories(coreengine PUBLIC include)
target_link_libraries(coreengine PUBLIC SDL3::SDL3)
target_compile_features(coreengine PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(coreengine PRIVATE /W4 /permissive-)
else()
  target_compile_options(coreengine PRIVATE -Wall -Wextra)
endif()
