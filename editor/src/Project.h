#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace editor {

enum class ProjectTemplate {
    Empty,
    Starter
};

struct Project {
    std::string name;
    std::string root;
    std::string scene = "scenes/main.scene";

    std::string FilePath() const;
    std::string ScenePath() const;
};

std::string PreferencesDirectory();
std::string DefaultProjectsDirectory();
bool ValidProjectName(const std::string& name);
bool ProjectExists(const std::string& root);

bool CreateProject(const std::string& parent, const std::string& name, ProjectTemplate tmpl,
                   Project& out, std::string& error);
bool LoadProject(const std::string& root, Project& out, std::string& error);
bool OpenOrCreateProject(const std::string& root, Project& out, std::string& error);

struct RecentEntry {
    std::string name;
    std::string root;
};

class RecentProjects {
public:
    void Load();
    void Save() const;
    void Add(const Project& project);
    void Remove(std::size_t index);
    const std::vector<RecentEntry>& Entries() const { return entries_; }

private:
    std::vector<RecentEntry> entries_;
};

}
