#pragma once

#include <string>
#include <vector>

#include "Project.h"
#include "core/Math.h"
#include "core/Renderer.h"
#include "core/SceneData.h"

struct SDL_Texture;

namespace editor {

struct AssetNode {
    std::string name;
    bool directory = false;
    std::vector<AssetNode> children;
};

class EditorScreen {
public:
    enum class Action {
        None,
        CloseProject,
        Quit
    };

    EditorScreen(core::Renderer& renderer, Project project, core::SceneData scene);
    ~EditorScreen();

    EditorScreen(const EditorScreen&) = delete;
    EditorScreen& operator=(const EditorScreen&) = delete;

    void Update(float dt);
    void PrepareViewport();
    Action Draw();

    bool Dirty() const { return dirty_; }
    void AskToQuit();
    std::string Title() const;

private:
    enum class Pending {
        None,
        Close,
        Quit
    };

    void BuildLayout(unsigned int dockId, float width, float height);
    void DrawMenuBar(Action& action);
    void DrawHierarchy();
    void DrawInspector();
    void DrawAssets();
    void DrawViewport();
    void DrawPrompt(Action& action);
    void DrawAbout();
    void DrawAssetNode(const AssetNode& node);

    void Request(Pending pending, Action& action);
    void RefreshAssets();
    bool Save();
    void Play();
    void Stop();
    void AddEntity(core::Shape shape);
    void DuplicateSelected();
    void DeleteSelected();
    void EnsureViewportTexture(int width, int height);
    core::Vec2 ViewportCenter() const;

    core::Renderer& renderer_;
    Project project_;
    core::SceneData scene_;
    core::SceneData snapshot_;

    SDL_Texture* viewportTexture_ = nullptr;
    int textureWidth_ = 0;
    int textureHeight_ = 0;
    core::Vec2 viewportSize_{640.0f, 360.0f};

    AssetNode assets_;
    float assetTimer_ = 0.0f;

    int selected_ = -1;
    bool playing_ = false;
    bool dirty_ = false;
    bool dragging_ = false;
    core::Vec2 dragOffset_;

    bool resetLayout_ = false;
    bool openPrompt_ = false;
    bool openAbout_ = false;
    Pending pending_ = Pending::None;
    std::string status_;
};

}
