add_executable(coreengine_editor
  src/EditorApp.cpp
  src/EditorScreen.cpp
  src/HomeScreen.cpp
  src/Project.cpp
  src/Theme.cpp
  src/main.cpp
)

target_link_libraries(coreengine_editor PRIVATE CoreEngine::Engine imgui)

coreengine_configure_app(coreengine_editor
  NAME "Core Engine"
  IDENTIFIER "com.coreengine.editor"
  OUTPUT_NAME "CoreEditor"
)
