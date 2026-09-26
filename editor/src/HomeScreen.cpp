#include "HomeScreen.h"

#include <SDL3/SDL.h>
#include <imgui.h>

#include <cfloat>
#include <cstdio>

#include "core/Icon.h"
#include "core/Window.h"

namespace editor {

namespace {

constexpr const char* kVersion = "0.1.0";

const ImVec4 kAccent(0.220f, 0.741f, 0.973f, 1.0f);
const ImVec4 kError(0.973f, 0.443f, 0.443f, 1.0f);

}

HomeScreen::HomeScreen(core::Window& window, SDL_Renderer* renderer, RecentProjects& recent)
    : window_(window), renderer_(renderer), recent_(recent) {
    const core::IconImage icon = core::DefaultIcon();
    logo_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, icon.width, icon.height);
    if (logo_ != nullptr) {
        SDL_UpdateTexture(logo_, nullptr, icon.pixels, icon.width * 4);
        SDL_SetTextureBlendMode(logo_, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(logo_, SDL_SCALEMODE_LINEAR);
    }

    const std::string defaultLocation = DefaultProjectsDirectory();
    std::snprintf(location_, sizeof(location_), "%s", defaultLocation.c_str());
    Refresh();
}

HomeScreen::~HomeScreen() {
    if (logo_ != nullptr) SDL_DestroyTexture(logo_);
}

void SDLCALL HomeScreen::OnFolderChosen(void* userdata, const char* const* files, int filter) {
    (void)filter;
    if (files == nullptr || files[0] == nullptr) return;
    auto* result = static_cast<DialogResult*>(userdata);
    std::lock_guard<std::mutex> lock(result->mutex);
    result->value = files[0];
    result->ready = true;
}

void HomeScreen::Refresh() {
    missing_.clear();
    for (const RecentEntry& entry : recent_.Entries()) {
        missing_.push_back(!ProjectExists(entry.root));
    }
}

void HomeScreen::SetError(const std::string& message) {
    error_ = message;
}

bool HomeScreen::TakeResult(DialogResult& result, std::string& value) {
    std::lock_guard<std::mutex> lock(result.mutex);
    if (!result.ready) return false;
    value = std::move(result.value);
    result.ready = false;
    return true;
}

void HomeScreen::DrawHeader(float unit) {
    const float logoSize = unit * 4.6f;
    if (logo_ != nullptr) {
        ImGui::Image(reinterpret_cast<ImTextureID>(logo_), ImVec2(logoSize, logoSize));
        ImGui::SameLine(0.0f, unit * 1.2f);
    }

    ImGui::BeginGroup();
    ImGui::SetWindowFontScale(2.1f);
    ImGui::TextUnformatted("Core Engine");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::TextColored(kAccent, "Version %s", kVersion);
    ImGui::TextDisabled("Create games and programs for desktop and mobile.");
    ImGui::EndGroup();
}

void HomeScreen::DrawRecent(HomeRequest& request, float unit) {
    ImGui::TextUnformatted("Projects");
    const char* openLabel = "Open Folder...";
    const float buttonWidth = ImGui::CalcTextSize(openLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonWidth + ImGui::GetCursorPosX());
    if (ImGui::Button(openLabel)) {
        SDL_ShowOpenFolderDialog(
            OnFolderChosen,
            &openResult_, window_.Handle(), nullptr, false);
    }

    ImGui::BeginChild("recent", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
    const auto& entries = recent_.Entries();
    if (entries.empty()) {
        ImGui::Dummy(ImVec2(0.0f, unit));
        ImGui::TextDisabled("No projects yet.");
        ImGui::TextDisabled("Create a new one or open an existing folder.");
    }

    int removeIndex = -1;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const RecentEntry& entry = entries[i];
        const bool missing = i < missing_.size() && missing_[i];
        ImGui::PushID(static_cast<int>(i));

        const float rowHeight = unit * 3.4f;
        const ImVec2 start = ImGui::GetCursorScreenPos();
        const float rowWidth = ImGui::GetContentRegionAvail().x;

        ImGui::BeginDisabled(missing);
        const bool activated = ImGui::Selectable("##row", false,
                                                 ImGuiSelectableFlags_AllowDoubleClick |
                                                     ImGuiSelectableFlags_AllowOverlap,
                                                 ImVec2(rowWidth, rowHeight));
        ImGui::EndDisabled();
        if (activated && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            request.kind = HomeRequest::Kind::Open;
            request.root = entry.root;
        }

        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImU32 titleColor = ImGui::GetColorU32(missing ? ImGuiCol_TextDisabled : ImGuiCol_Text);
        draw->AddText(ImVec2(start.x + unit * 0.6f, start.y + unit * 0.4f), titleColor, entry.name.c_str());
        const std::string subtitle = missing ? entry.root + "  (missing)" : entry.root;
        draw->AddText(ImVec2(start.x + unit * 0.6f, start.y + unit * 1.7f),
                      ImGui::GetColorU32(ImGuiCol_TextDisabled), subtitle.c_str());

        const float removeWidth = ImGui::CalcTextSize("Remove").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float openWidth = ImGui::CalcTextSize("Open").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const float rightEdge = start.x + rowWidth - unit * 0.4f;
        const float buttonY = start.y + (rowHeight - ImGui::GetFrameHeight()) * 0.5f;

        ImGui::SetCursorScreenPos(ImVec2(rightEdge - removeWidth, buttonY));
        if (ImGui::Button("Remove")) removeIndex = static_cast<int>(i);

        ImGui::SetCursorScreenPos(ImVec2(rightEdge - removeWidth - spacing - openWidth, buttonY));
        ImGui::BeginDisabled(missing);
        if (ImGui::Button("Open")) {
            request.kind = HomeRequest::Kind::Open;
            request.root = entry.root;
        }
        ImGui::EndDisabled();

        ImGui::SetCursorScreenPos(ImVec2(start.x, start.y + rowHeight + ImGui::GetStyle().ItemSpacing.y));
        ImGui::PopID();
    }
    ImGui::EndChild();

    if (removeIndex >= 0) {
        recent_.Remove(static_cast<std::size_t>(removeIndex));
        Refresh();
    }
}

void HomeScreen::DrawCreate(HomeRequest& request, float unit) {
    (void)unit;
    ImGui::TextUnformatted("New Project");
    ImGui::BeginChild("create", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

    ImGui::TextDisabled("Name");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##name", name_, sizeof(name_));

    ImGui::Spacing();
    ImGui::TextDisabled("Location");
    const float browseWidth = ImGui::CalcTextSize("Browse...").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - browseWidth - ImGui::GetStyle().ItemSpacing.x);
    ImGui::InputText("##location", location_, sizeof(location_));
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        SDL_ShowOpenFolderDialog(
            OnFolderChosen,
            &browseResult_, window_.Handle(), location_, false);
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Template");
    const char* templates[] = {"Empty", "2D Starter"};
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::Combo("##template", &templateIndex_, templates, 2);

    ImGui::Spacing();
    ImGui::TextDisabled("Project folder");
    ImGui::TextWrapped("%s/%s", location_, name_);

    ImGui::Spacing();
    if (!error_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, kError);
        ImGui::TextWrapped("%s", error_.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, kAccent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.376f, 0.800f, 0.984f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.130f, 0.620f, 0.860f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.06f, 0.10f, 1.0f));
    const bool create = ImGui::Button("Create Project", ImVec2(-FLT_MIN, 0.0f));
    ImGui::PopStyleColor(4);

    if (create) {
        Project project;
        std::string error;
        const ProjectTemplate tmpl = templateIndex_ == 0 ? ProjectTemplate::Empty : ProjectTemplate::Starter;
        if (CreateProject(location_, name_, tmpl, project, error)) {
            error_.clear();
            request.kind = HomeRequest::Kind::Open;
            request.root = project.root;
        } else {
            error_ = error;
        }
    }
    ImGui::EndChild();
}

HomeRequest HomeScreen::Draw() {
    HomeRequest request;

    std::string picked;
    if (TakeResult(openResult_, picked)) {
        request.kind = HomeRequest::Kind::Open;
        request.root = picked;
    }
    if (TakeResult(browseResult_, picked)) {
        std::snprintf(location_, sizeof(location_), "%s", picked.c_str());
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ImGui::GetFontSize() * 2.2f, ImGui::GetFontSize() * 1.8f));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                   ImGuiWindowFlags_NoDocking;
    ImGui::Begin("Home", nullptr, flags);
    ImGui::PopStyleVar(3);

    const float unit = ImGui::GetFontSize();
    DrawHeader(unit);
    ImGui::Dummy(ImVec2(0.0f, unit * 0.8f));

    if (ImGui::BeginTable("layout", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("projects", ImGuiTableColumnFlags_WidthStretch, 1.7f);
        ImGui::TableSetupColumn("create", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableNextRow();
        const float height = ImGui::GetContentRegionAvail().y - unit * 2.0f;
        ImGui::TableSetColumnIndex(0);
        ImGui::BeginChild("left", ImVec2(0.0f, height));
        DrawRecent(request, unit);
        ImGui::EndChild();
        ImGui::TableSetColumnIndex(1);
        ImGui::BeginChild("right", ImVec2(0.0f, height));
        DrawCreate(request, unit);
        ImGui::EndChild();
        ImGui::EndTable();
    }

    ImGui::End();
    return request;
}

}
