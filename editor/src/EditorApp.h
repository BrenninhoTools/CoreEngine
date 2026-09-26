#pragma once

#include <memory>
#include <string>

#include "EditorScreen.h"
#include "HomeScreen.h"
#include "Project.h"
#include "core/Application.h"

namespace editor {

class EditorApp : public core::Application {
public:
    EditorApp(core::ApplicationConfig config, std::string startupProject);

protected:
    void OnStart() override;
    void OnEvent(const SDL_Event& event) override;
    bool OnQuitRequested() override;
    void OnUpdate(float dt) override;
    void OnRender(core::Renderer& renderer) override;
    void OnShutdown() override;

private:
    bool OpenProject(const std::string& root);
    void CloseProject();
    void CreateInterface();
    void DestroyInterface();

    std::string startupProject_;
    std::string interfaceIni_;
    std::string title_;
    RecentProjects recent_;
    std::unique_ptr<HomeScreen> home_;
    std::unique_ptr<EditorScreen> editor_;
    bool interfaceReady_ = false;
};

}
