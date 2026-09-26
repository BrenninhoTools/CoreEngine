#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "core/Input.h"
#include "core/Math.h"
#include "core/Renderer.h"
#include "core/Scene.h"
#include "core/Window.h"

namespace core {

struct ApplicationConfig {
    std::string name = "Core Engine";
    std::string identifier = "com.coreengine.app";
    std::string version = "0.1.0";
    WindowConfig window;
    bool vsync = true;
    bool useDefaultIcon = true;
    Color clearColor{16, 18, 28, 255};
    float fixedStep = 1.0f / 60.0f;
    std::uint64_t maxFrames = 0;
};

class Application {
public:
    explicit Application(ApplicationConfig config = {});
    virtual ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run();
    void Quit();

    template <typename T, typename... Args>
    T& SetScene(Args&&... args) {
        static_assert(std::is_base_of_v<Scene, T>, "T must derive from core::Scene");
        auto scene = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *scene;
        pendingScene_ = std::move(scene);
        return ref;
    }

    Window& GetWindow() { return window_; }
    Renderer& GetRenderer() { return renderer_; }
    Input& GetInput() { return input_; }
    const ApplicationConfig& Config() const { return config_; }

    std::uint64_t FrameCount() const { return frame_; }
    double TotalTime() const { return totalTime_; }
    bool Paused() const { return paused_; }

protected:
    virtual void OnStart() {}
    virtual void OnEvent(const SDL_Event& event) { (void)event; }
    virtual bool OnQuitRequested() { return true; }
    virtual void OnFixedUpdate(float dt) { (void)dt; }
    virtual void OnUpdate(float dt) { (void)dt; }
    virtual void OnRender(Renderer& renderer) { (void)renderer; }
    virtual void OnPause() {}
    virtual void OnResume() {}
    virtual void OnLowMemory() {}
    virtual void OnShutdown() {}

private:
    bool Initialize();
    void Shutdown();
    void PumpEvents();
    void ApplyPendingScene();
    void Step(float dt);
    void Draw();

    ApplicationConfig config_;
    Window window_;
    Renderer renderer_;
    Input input_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<Scene> pendingScene_;
    bool sdlReady_ = false;
    bool running_ = false;
    bool paused_ = false;
    std::uint64_t frame_ = 0;
    double totalTime_ = 0.0;
    double accumulator_ = 0.0;
};

}
