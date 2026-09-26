#include "EditorScreen.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace editor {

namespace {

constexpr float kGridStep = 64.0f;
constexpr float kAssetRefreshSeconds = 3.0f;
constexpr int kAssetMaxDepth = 6;
constexpr int kAssetMaxNodes = 2000;

const core::Color kAccent{56, 189, 248, 255};

const core::Color kPalette[] = {
    {56, 189, 248, 255},
    {250, 204, 21, 255},
    {248, 113, 113, 255},
    {74, 222, 128, 255},
    {192, 132, 252, 255},
    {251, 146, 60, 255}};

void ScanDirectory(const fs::path& path, AssetNode& node, int depth, int& budget) {
    std::error_code ec;
    fs::directory_iterator it(path, fs::directory_options::skip_permission_denied, ec);
    if (ec) return;

    for (const fs::directory_entry& entry : it) {
        if (budget <= 0) break;
        const std::string name = entry.path().filename().u8string();
        if (name.empty() || name[0] == '.') continue;

        --budget;
        AssetNode child;
        child.name = name;
        child.directory = entry.is_directory(ec);
        if (child.directory && depth < kAssetMaxDepth) {
            ScanDirectory(entry.path(), child, depth + 1, budget);
        }
        node.children.push_back(std::move(child));
    }

    std::sort(node.children.begin(), node.children.end(), [](const AssetNode& a, const AssetNode& b) {
        if (a.directory != b.directory) return a.directory;
        return a.name < b.name;
    });
}

bool EditColor(const char* label, core::Color& color) {
    float value[4] = {
        static_cast<float>(color.r) / 255.0f,
        static_cast<float>(color.g) / 255.0f,
        static_cast<float>(color.b) / 255.0f,
        static_cast<float>(color.a) / 255.0f};
    if (!ImGui::ColorEdit4(label, value)) return false;
    color.r = static_cast<std::uint8_t>(core::Clamp(value[0], 0.0f, 1.0f) * 255.0f + 0.5f);
    color.g = static_cast<std::uint8_t>(core::Clamp(value[1], 0.0f, 1.0f) * 255.0f + 0.5f);
    color.b = static_cast<std::uint8_t>(core::Clamp(value[2], 0.0f, 1.0f) * 255.0f + 0.5f);
    color.a = static_cast<std::uint8_t>(core::Clamp(value[3], 0.0f, 1.0f) * 255.0f + 0.5f);
    return true;
}

bool EditText(const char* label, std::string& value) {
    char buffer[128];
    std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());
    if (!ImGui::InputText(label, buffer, sizeof(buffer))) return false;
    value = buffer;
    return true;
}

EditorScreen::Action ToAction(int pending) {
    return pending == 1 ? EditorScreen::Action::CloseProject : EditorScreen::Action::Quit;
}

}

EditorScreen::EditorScreen(core::Renderer& renderer, Project project, core::SceneData scene)
    : renderer_(renderer), project_(std::move(project)), scene_(std::move(scene)) {
    RefreshAssets();
}

EditorScreen::~EditorScreen() {
    if (viewportTexture_ != nullptr) SDL_DestroyTexture(viewportTexture_);
}

std::string EditorScreen::Title() const {
    return project_.name + (dirty_ ? " *" : "") + " - Core Engine";
}

void EditorScreen::AskToQuit() {
    pending_ = Pending::Quit;
    openPrompt_ = true;
}

void EditorScreen::Request(Pending pending, Action& action) {
    if (dirty_) {
        pending_ = pending;
        openPrompt_ = true;
        return;
    }
    action = pending == Pending::Close ? Action::CloseProject : Action::Quit;
}

void EditorScreen::RefreshAssets() {
    assets_ = AssetNode{};
    assets_.name = project_.name;
    assets_.directory = true;
    int budget = kAssetMaxNodes;
    ScanDirectory(fs::u8path(project_.root), assets_, 0, budget);
    assetTimer_ = 0.0f;
}

bool EditorScreen::Save() {
    if (playing_) Stop();
    if (!scene_.Save(project_.ScenePath())) {
        status_ = "Save failed";
        return false;
    }
    dirty_ = false;
    status_ = "Saved";
    RefreshAssets();
    return true;
}

void EditorScreen::Play() {
    if (playing_) return;
    snapshot_ = scene_;
    playing_ = true;
    dragging_ = false;
    status_ = "Playing";
}

void EditorScreen::Stop() {
    if (!playing_) return;
    scene_ = snapshot_;
    playing_ = false;
    status_.clear();
}

core::Vec2 EditorScreen::ViewportCenter() const {
    return {viewportSize_.x * 0.5f, viewportSize_.y * 0.5f};
}

void EditorScreen::AddEntity(core::Shape shape) {
    core::EntityData entity;
    const bool circle = shape == core::Shape::Circle;
    entity.name = std::string(circle ? "Circle " : "Rectangle ") + std::to_string(scene_.entities.size() + 1);
    entity.shape = shape;
    entity.size = circle ? core::Vec2{64.0f, 64.0f} : core::Vec2{96.0f, 64.0f};
    entity.position = ViewportCenter();
    entity.color = kPalette[scene_.entities.size() % (sizeof(kPalette) / sizeof(kPalette[0]))];
    scene_.entities.push_back(entity);
    selected_ = static_cast<int>(scene_.entities.size()) - 1;
    dirty_ = true;
}

void EditorScreen::DuplicateSelected() {
    if (selected_ < 0 || selected_ >= static_cast<int>(scene_.entities.size())) return;
    core::EntityData copy = scene_.entities[static_cast<std::size_t>(selected_)];
    copy.name += " Copy";
    copy.position += core::Vec2{24.0f, 24.0f};
    scene_.entities.push_back(copy);
    selected_ = static_cast<int>(scene_.entities.size()) - 1;
    dirty_ = true;
}

void EditorScreen::DeleteSelected() {
    if (selected_ < 0 || selected_ >= static_cast<int>(scene_.entities.size())) return;
    scene_.entities.erase(scene_.entities.begin() + selected_);
    selected_ = std::min(selected_, static_cast<int>(scene_.entities.size()) - 1);
    dirty_ = true;
}

void EditorScreen::Update(float dt) {
    if (playing_) {
        scene_.Step(dt, core::Rect{0.0f, 0.0f, viewportSize_.x, viewportSize_.y});
    }
    assetTimer_ += dt;
    if (assetTimer_ >= kAssetRefreshSeconds) RefreshAssets();
}

void EditorScreen::EnsureViewportTexture(int width, int height) {
    if (viewportTexture_ != nullptr && textureWidth_ == width && textureHeight_ == height) return;
    if (viewportTexture_ != nullptr) SDL_DestroyTexture(viewportTexture_);
    viewportTexture_ = SDL_CreateTexture(renderer_.Handle(), SDL_PIXELFORMAT_RGBA32,
                                         SDL_TEXTUREACCESS_TARGET, width, height);
    textureWidth_ = width;
    textureHeight_ = height;
    if (viewportTexture_ != nullptr) SDL_SetTextureScaleMode(viewportTexture_, SDL_SCALEMODE_LINEAR);
}

void EditorScreen::PrepareViewport() {
    const float scale = std::max(1.0f, ImGui::GetIO().DisplayFramebufferScale.x);
    const int width = std::max(16, static_cast<int>(viewportSize_.x * scale));
    const int height = std::max(16, static_cast<int>(viewportSize_.y * scale));
    EnsureViewportTexture(width, height);
    if (viewportTexture_ == nullptr) return;

    SDL_Renderer* handle = renderer_.Handle();
    SDL_Texture* previous = SDL_GetRenderTarget(handle);
    SDL_SetRenderTarget(handle, viewportTexture_);
    SDL_SetRenderScale(handle, scale, scale);

    renderer_.BeginFrame(scene_.background);

    const core::Color grid{255, 255, 255, 14};
    for (float x = 0.0f; x < viewportSize_.x; x += kGridStep) {
        renderer_.DrawLine({x, 0.0f}, {x, viewportSize_.y}, grid);
    }
    for (float y = 0.0f; y < viewportSize_.y; y += kGridStep) {
        renderer_.DrawLine({0.0f, y}, {viewportSize_.x, y}, grid);
    }

    scene_.Draw(renderer_);

    if (!playing_ && selected_ >= 0 && selected_ < static_cast<int>(scene_.entities.size())) {
        core::Rect bounds = scene_.entities[static_cast<std::size_t>(selected_)].Bounds();
        bounds.x -= 3.0f;
        bounds.y -= 3.0f;
        bounds.w += 6.0f;
        bounds.h += 6.0f;
        renderer_.DrawRect(bounds, kAccent);
    }

    SDL_SetRenderScale(handle, 1.0f, 1.0f);
    SDL_SetRenderTarget(handle, previous);
}

void EditorScreen::BuildLayout(unsigned int dockId, float width, float height) {
    ImGui::DockBuilderRemoveNode(dockId);
    ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockId, ImVec2(width, height));

    ImGuiID center = dockId;
    const ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.20f, nullptr, &center);
    const ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.26f, nullptr, &center);
    const ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.28f, nullptr, &center);

    ImGui::DockBuilderDockWindow("Hierarchy", left);
    ImGui::DockBuilderDockWindow("Inspector", right);
    ImGui::DockBuilderDockWindow("Assets", bottom);
    ImGui::DockBuilderDockWindow("Viewport", center);
    ImGui::DockBuilderFinish(dockId);
}

void EditorScreen::DrawMenuBar(Action& action) {
    if (!ImGui::BeginMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save", "Ctrl+S", false, !playing_)) Save();
        ImGui::Separator();
        if (ImGui::MenuItem("Close Project")) Request(Pending::Close, action);
        if (ImGui::MenuItem("Exit")) Request(Pending::Quit, action);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Add Rectangle", nullptr, false, !playing_)) AddEntity(core::Shape::Rect);
        if (ImGui::MenuItem("Add Circle", nullptr, false, !playing_)) AddEntity(core::Shape::Circle);
        ImGui::Separator();
        const bool hasSelection = selected_ >= 0 && selected_ < static_cast<int>(scene_.entities.size());
        if (ImGui::MenuItem("Duplicate", nullptr, false, hasSelection && !playing_)) DuplicateSelected();
        if (ImGui::MenuItem("Delete", "Del", false, hasSelection && !playing_)) DeleteSelected();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Scene")) {
        if (ImGui::MenuItem("Play", nullptr, false, !playing_)) Play();
        if (ImGui::MenuItem("Stop", nullptr, false, playing_)) Stop();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Reset Layout")) resetLayout_ = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About Core Engine")) openAbout_ = true;
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

void EditorScreen::DrawHierarchy() {
    ImGui::Begin("Hierarchy");

    ImGui::BeginDisabled(playing_);
    if (ImGui::Button("+ Rectangle")) AddEntity(core::Shape::Rect);
    ImGui::SameLine();
    if (ImGui::Button("+ Circle")) AddEntity(core::Shape::Circle);
    ImGui::EndDisabled();
    ImGui::Separator();

    int duplicate = -1;
    int remove = -1;
    for (int i = 0; i < static_cast<int>(scene_.entities.size()); ++i) {
        const core::EntityData& entity = scene_.entities[static_cast<std::size_t>(i)];
        ImGui::PushID(i);
        if (ImGui::Selectable(entity.name.c_str(), selected_ == i)) selected_ = i;
        if (!playing_ && ImGui::BeginPopupContextItem()) {
            selected_ = i;
            if (ImGui::MenuItem("Duplicate")) duplicate = i;
            if (ImGui::MenuItem("Delete")) remove = i;
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    if (scene_.entities.empty()) ImGui::TextDisabled("The scene is empty.");

    if (duplicate >= 0) DuplicateSelected();
    if (remove >= 0) DeleteSelected();

    ImGui::End();
}

void EditorScreen::DrawInspector() {
    ImGui::Begin("Inspector");
    ImGui::BeginDisabled(playing_);

    if (selected_ >= 0 && selected_ < static_cast<int>(scene_.entities.size())) {
        core::EntityData& entity = scene_.entities[static_cast<std::size_t>(selected_)];
        bool changed = false;

        changed |= EditText("Name", entity.name);

        int shape = entity.shape == core::Shape::Circle ? 1 : 0;
        const char* shapes[] = {"Rectangle", "Circle"};
        if (ImGui::Combo("Shape", &shape, shapes, 2)) {
            entity.shape = shape == 1 ? core::Shape::Circle : core::Shape::Rect;
            if (shape == 1) entity.size.y = entity.size.x;
            changed = true;
        }

        changed |= ImGui::DragFloat2("Position", &entity.position.x, 1.0f);

        if (entity.shape == core::Shape::Circle) {
            if (ImGui::DragFloat("Diameter", &entity.size.x, 1.0f, 1.0f, 4096.0f)) {
                entity.size.y = entity.size.x;
                changed = true;
            }
        } else {
            changed |= ImGui::DragFloat2("Size", &entity.size.x, 1.0f, 1.0f, 4096.0f);
        }

        changed |= EditColor("Color", entity.color);
        changed |= ImGui::DragFloat2("Velocity", &entity.velocity.x, 1.0f, -2000.0f, 2000.0f);

        if (changed) dirty_ = true;
    } else {
        ImGui::TextDisabled("Scene");
        ImGui::Separator();
        bool changed = EditText("Name", scene_.name);
        changed |= EditColor("Background", scene_.background);
        if (changed) dirty_ = true;
        ImGui::Spacing();
        ImGui::TextDisabled("%d entities", static_cast<int>(scene_.entities.size()));
    }

    ImGui::EndDisabled();
    ImGui::End();
}

void EditorScreen::DrawAssetNode(const AssetNode& node) {
    if (node.directory) {
        if (ImGui::TreeNodeEx(node.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
            for (const AssetNode& child : node.children) DrawAssetNode(child);
            ImGui::TreePop();
        }
    } else {
        ImGui::TreeNodeEx(node.name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                                 ImGuiTreeNodeFlags_SpanAvailWidth);
    }
}

void EditorScreen::DrawAssets() {
    ImGui::Begin("Assets");
    if (ImGui::Button("Refresh")) RefreshAssets();
    ImGui::SameLine();
    ImGui::TextDisabled("%s", project_.root.c_str());
    ImGui::Separator();

    ImGui::BeginChild("assetTree");
    for (const AssetNode& child : assets_.children) DrawAssetNode(child);
    ImGui::EndChild();
    ImGui::End();
}

void EditorScreen::DrawViewport() {
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (playing_) {
        if (ImGui::Button("Stop")) Stop();
    } else {
        if (ImGui::Button("Play")) Play();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(playing_);
    if (ImGui::Button("Save")) Save();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("%s%s%s%s", scene_.name.c_str(), dirty_ ? " *" : "", status_.empty() ? "" : "  |  ",
                        status_.c_str());

    ImVec2 available = ImGui::GetContentRegionAvail();
    available.x = std::max(available.x, 16.0f);
    available.y = std::max(available.y, 16.0f);
    viewportSize_ = {available.x, available.y};

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    if (viewportTexture_ != nullptr) {
        ImGui::Image(reinterpret_cast<ImTextureID>(viewportTexture_), available);
    }
    ImGui::SetCursorScreenPos(origin);
    ImGui::InvisibleButton("##surface", available, ImGuiButtonFlags_MouseButtonLeft);

    const ImVec2 mouseAbsolute = ImGui::GetIO().MousePos;
    const core::Vec2 mouse{mouseAbsolute.x - origin.x, mouseAbsolute.y - origin.y};

    if (!playing_) {
        if (ImGui::IsItemActivated()) {
            selected_ = scene_.Pick(mouse);
            dragging_ = selected_ >= 0;
            if (dragging_) dragOffset_ = scene_.entities[static_cast<std::size_t>(selected_)].position - mouse;
        }
        if (dragging_ && ImGui::IsItemActive() && selected_ >= 0 &&
            selected_ < static_cast<int>(scene_.entities.size())) {
            core::EntityData& entity = scene_.entities[static_cast<std::size_t>(selected_)];
            const core::Vec2 target = mouse + dragOffset_;
            if (target.x != entity.position.x || target.y != entity.position.y) {
                entity.position = target;
                dirty_ = true;
            }
        }
        if (!ImGui::IsItemActive()) dragging_ = false;
    }

    ImGui::End();
}

void EditorScreen::DrawAbout() {
    if (openAbout_) {
        ImGui::OpenPopup("About Core Engine");
        openAbout_ = false;
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("About Core Engine", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Core Engine 0.1.0");
        ImGui::TextDisabled("A cross-platform engine for games and programs.");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(-FLT_MIN, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void EditorScreen::DrawPrompt(Action& action) {
    if (openPrompt_) {
        ImGui::OpenPopup("Unsaved Changes");
        openPrompt_ = false;
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Save changes to \"%s\" before continuing?", project_.name.c_str());
        ImGui::Spacing();

        const int pending = pending_ == Pending::Close ? 1 : 2;
        if (ImGui::Button("Save")) {
            if (Save()) {
                action = ToAction(pending);
                pending_ = Pending::None;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Don't Save")) {
            dirty_ = false;
            action = ToAction(pending);
            pending_ = Pending::None;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            pending_ = Pending::None;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

EditorScreen::Action EditorScreen::Draw() {
    Action action = Action::None;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                   ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("EditorHost", nullptr, flags);
    ImGui::PopStyleVar(3);

    DrawMenuBar(action);

    const ImGuiID dockId = ImGui::GetID("CoreEditorDock");
    if (resetLayout_ || ImGui::DockBuilderGetNode(dockId) == nullptr) {
        BuildLayout(dockId, viewport->WorkSize.x, viewport->WorkSize.y);
        resetLayout_ = false;
    }
    ImGui::DockSpace(dockId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();

    DrawHierarchy();
    DrawInspector();
    DrawAssets();
    DrawViewport();

    const ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S) && !playing_) Save();
    if (!io.WantTextInput && !playing_ && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) DeleteSelected();

    DrawAbout();
    DrawPrompt(action);
    return action;
}

}
