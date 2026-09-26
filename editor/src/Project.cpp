#include "Project.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

#include "core/SceneData.h"

namespace fs = std::filesystem;

namespace editor {

namespace {

constexpr const char* kProjectFile = "project.core";
constexpr const char* kProjectHeader = "core-project 1";
constexpr const char* kEngineVersion = "0.1.0";
constexpr std::size_t kMaxRecent = 12;

fs::path ToPath(const std::string& text) {
    return fs::u8path(text);
}

std::string FromPath(const fs::path& path) {
    return path.u8string();
}

core::SceneData StarterScene(const std::string& name) {
    core::SceneData scene;
    scene.name = name;

    core::EntityData player;
    player.name = "Player";
    player.position = {240.0f, 220.0f};
    player.size = {72.0f, 72.0f};
    player.color = {56, 189, 248, 255};
    scene.entities.push_back(player);

    core::EntityData sun;
    sun.name = "Sun";
    sun.shape = core::Shape::Circle;
    sun.position = {520.0f, 160.0f};
    sun.size = {96.0f, 96.0f};
    sun.color = {250, 204, 21, 255};
    sun.velocity = {90.0f, 60.0f};
    scene.entities.push_back(sun);

    core::EntityData ball;
    ball.name = "Ball";
    ball.shape = core::Shape::Circle;
    ball.position = {400.0f, 320.0f};
    ball.size = {56.0f, 56.0f};
    ball.color = {248, 113, 113, 255};
    ball.velocity = {-120.0f, 80.0f};
    scene.entities.push_back(ball);

    core::EntityData block;
    block.name = "Block";
    block.position = {640.0f, 300.0f};
    block.size = {120.0f, 48.0f};
    block.color = {74, 222, 128, 255};
    block.velocity = {60.0f, 0.0f};
    scene.entities.push_back(block);

    return scene;
}

bool WriteProjectFile(const Project& project) {
    std::ofstream file(ToPath(project.FilePath()), std::ios::binary | std::ios::trunc);
    if (!file) return false;
    file << kProjectHeader << '\n';
    file << "name " << project.name << '\n';
    file << "engine " << kEngineVersion << '\n';
    file << "scene " << project.scene << '\n';
    return static_cast<bool>(file);
}

std::string Trim(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return {};
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

}

std::string Project::FilePath() const {
    return FromPath(ToPath(root) / kProjectFile);
}

std::string Project::ScenePath() const {
    return FromPath(ToPath(root) / ToPath(scene));
}

std::string PreferencesDirectory() {
    char* path = SDL_GetPrefPath("CoreEngine", "Editor");
    if (path == nullptr) return "./";
    std::string result = path;
    SDL_free(path);
    return result;
}

std::string DefaultProjectsDirectory() {
    const char* documents = SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS);
    const fs::path base = documents != nullptr ? ToPath(documents) : ToPath(PreferencesDirectory());
    return FromPath(base / "CoreEngineProjects");
}

bool ValidProjectName(const std::string& name) {
    const std::string trimmed = Trim(name);
    if (trimmed.empty() || trimmed != name || name == "." || name == "..") return false;
    return name.find_first_of("\\/:*?\"<>|") == std::string::npos;
}

bool ProjectExists(const std::string& root) {
    std::error_code ec;
    return fs::is_regular_file(ToPath(root) / kProjectFile, ec);
}

bool CreateProject(const std::string& parent, const std::string& name, ProjectTemplate tmpl,
                   Project& out, std::string& error) {
    if (!ValidProjectName(name)) {
        error = "Enter a valid project name without special characters.";
        return false;
    }
    if (Trim(parent).empty()) {
        error = "Choose a location for the project.";
        return false;
    }

    Project project;
    project.name = name;
    project.root = FromPath(ToPath(parent) / ToPath(name));

    if (ProjectExists(project.root)) {
        error = "A project already exists in that folder.";
        return false;
    }

    std::error_code ec;
    fs::create_directories(ToPath(project.root) / "scenes", ec);
    if (!ec) fs::create_directories(ToPath(project.root) / "assets", ec);
    if (ec) {
        error = "Could not create the project folder.";
        return false;
    }

    core::SceneData scene = tmpl == ProjectTemplate::Starter ? StarterScene(name) : core::SceneData{};
    scene.name = name;

    if (!WriteProjectFile(project) || !scene.Save(project.ScenePath())) {
        error = "Could not write the project files.";
        return false;
    }

    out = std::move(project);
    return true;
}

bool LoadProject(const std::string& root, Project& out, std::string& error) {
    std::ifstream file(ToPath(root) / kProjectFile, std::ios::binary);
    std::string line;
    if (!file || !std::getline(file, line) || Trim(line) != kProjectHeader) {
        error = "No Core Engine project was found in that folder.";
        return false;
    }

    Project project;
    project.root = root;
    while (std::getline(file, line)) {
        std::istringstream in(line);
        std::string key;
        in >> key;
        std::string value;
        std::getline(in, value);
        value = Trim(value);
        if (key == "name") project.name = value;
        else if (key == "scene" && !value.empty()) project.scene = value;
    }

    if (project.name.empty()) project.name = FromPath(ToPath(root).filename());
    out = std::move(project);
    return true;
}

bool OpenOrCreateProject(const std::string& root, Project& out, std::string& error) {
    if (ProjectExists(root)) return LoadProject(root, out, error);
    const fs::path path = ToPath(root);
    const std::string name = FromPath(path.filename());
    return CreateProject(FromPath(path.parent_path()), name, ProjectTemplate::Starter, out, error);
}

void RecentProjects::Load() {
    entries_.clear();
    std::ifstream file(ToPath(PreferencesDirectory()) / "recent.txt", std::ios::binary);
    std::string line;
    while (std::getline(file, line)) {
        const auto tab = line.find('\t');
        if (tab == std::string::npos) continue;
        RecentEntry entry{Trim(line.substr(tab + 1)), Trim(line.substr(0, tab))};
        if (!entry.root.empty()) entries_.push_back(std::move(entry));
    }
    if (entries_.size() > kMaxRecent) entries_.resize(kMaxRecent);
}

void RecentProjects::Save() const {
    std::ofstream file(ToPath(PreferencesDirectory()) / "recent.txt", std::ios::binary | std::ios::trunc);
    if (!file) return;
    for (const RecentEntry& entry : entries_) {
        file << entry.root << '\t' << entry.name << '\n';
    }
}

void RecentProjects::Add(const Project& project) {
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
                                  [&](const RecentEntry& e) { return e.root == project.root; }),
                   entries_.end());
    entries_.insert(entries_.begin(), RecentEntry{project.name, project.root});
    if (entries_.size() > kMaxRecent) entries_.resize(kMaxRecent);
    Save();
}

void RecentProjects::Remove(std::size_t index) {
    if (index >= entries_.size()) return;
    entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(index));
    Save();
}

}
