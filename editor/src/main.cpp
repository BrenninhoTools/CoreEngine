#include <cstdlib>
#include <cstring>
#include <string>

#include "EditorApp.h"
#include "core/EntryPoint.h"

int main(int argc, char** argv) {
    core::ApplicationConfig config;
    config.name = "Core Engine";
    config.identifier = "com.coreengine.editor";
    config.window.title = "Core Engine";
    config.window.width = 1440;
    config.window.height = 900;
    config.clearColor = {11, 13, 22, 255};

    std::string startupProject;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            config.maxFrames = std::strtoull(argv[++i], nullptr, 10);
        } else if (std::strcmp(argv[i], "--open") == 0 && i + 1 < argc) {
            startupProject = argv[++i];
        }
    }

    editor::EditorApp app(config, startupProject);
    return app.Run();
}
