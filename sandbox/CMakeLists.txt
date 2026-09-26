if(ANDROID)
  add_library(coreengine_sandbox SHARED src/main.cpp)
else()
  add_executable(coreengine_sandbox src/main.cpp)
endif()

target_link_libraries(coreengine_sandbox PRIVATE CoreEngine::Engine)

coreengine_configure_app(coreengine_sandbox
  NAME "Core Engine Sandbox"
  IDENTIFIER "com.coreengine.sandbox"
  OUTPUT_NAME "CoreEngineSandbox"
)
