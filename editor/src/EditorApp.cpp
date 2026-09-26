#include "EditorApp.h"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <imgui.h>

#include "Theme.h"
#include "core/SceneData.h"

namespace editor {

namespace {

constexpr const char* kHomeTitle = "Core Engine";

}

EditorApp::EditorApp(core::ApplicationConfig config, std::string startupProject)
    : core::Application(std::move(config)), startupProject_(std::move(startupProject)) {}

void EditorApp::CreateInterface() {
    SDL_Window* window = GetWindow().Handle();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    interfaceIni_ = PreferencesDirectory() + "layout.ini";
    io.IniFilename = interfaceIni_.c_str();

    float contentScale = SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(window));
    if (contentScale <= 0.0f) contentScale = 1.0f;
    float density = SDL_GetWindowPixelDensity(window);
    if (density <= 0.0f) density = 1.0f;

    ImFontConfig font;
    font.SizePixels = 15.0f * contentScale;
    font.RasterizerDensity = density;
    io.Fonts->AddFontDefault(&font);

    ApplyTheme(contentScale);

    ImGui_ImplSDL3_InitForSDLRenderer(window, GetRenderer().Handle());
    ImGui_ImplSDLRenderer3_Init(GetRenderer().Handle());
    interfaceReady_ = true;
}

void EditorApp::DestroyInterface() {
    if (!interfaceReady_) return;
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    interfaceReady_ = false;
}

void EditorApp::OnStart() {
    CreateInterface();
    recent_.Load();
    home_ = std::make_unique<HomeScreen>(GetWindow(), GetRenderer().Handle(), recent_);

    if (!startupProject_.empty()) {
        std::string error;
        Project project;
        if (OpenOrCreateProject(startupProject_, project, error)) {
            OpenProject(project.root);
        } else {
            home_->SetError(error);
        }
    }
}

bool EditorApp::OpenProject(const std::string& root) {
    Project project;
    std::string error;
    if (!LoadProject(root, project, error)) {
        home_->SetError(error);
        return false;
    }

    core::SceneData scene;
    if (!core::SceneData::Load(project.ScenePath(), scene)) {
        home_->SetError("The project scene could not be loaded.");
        return false;
    }

    recent_.Add(project);
    home_->Refresh();
    editor_ = std::make_unique<EditorScreen>(GetRenderer(), std::move(project), std::move(scene));
    return true;
}

void EditorApp::CloseProject() {
    editor_.reset();
    home_->Refresh();
}

void EditorApp::OnEvent(const SDL_Event& event) {
    if (interfaceReady_) ImGui_ImplSDL3_ProcessEvent(&event);
}

bool EditorApp::OnQuitRequested() {
    if (editor_ && editor_->Dirty()) {
        editor_->AskToQuit();
        return false;
    }
    return true;
}

void EditorApp::OnUpdate(float dt) {
    if (editor_) editor_->Update(dt);

    const std::string wanted = editor_ ? editor_->Title() : kHomeTitle;
    if (wanted != title_) {
        title_ = wanted;
        GetWindow().SetTitle(title_);
    }
}

void EditorApp::OnRender(core::Renderer& renderer) {
    if (editor_) editor_->PrepareViewport();

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    EditorScreen::Action action = EditorScreen::Action::None;
    HomeRequest request;
    if (editor_) {
        action = editor_->Draw();
    } else {
        request = home_->Draw();
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer.Handle());

    if (request.kind == HomeRequest::Kind::Open) OpenProject(request.root);
    if (action == EditorScreen::Action::CloseProject) CloseProject();
    if (action == EditorScreen::Action::Quit) Quit();
}

void EditorApp::OnShutdown() {
    editor_.reset();
    home_.reset();
    DestroyInterface();
}

}
