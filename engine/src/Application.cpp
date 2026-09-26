#include "core/Application.h"

#include <SDL3/SDL.h>

#include "core/Icon.h"

namespace core {

namespace {

constexpr double kMaxFrameTime = 0.25;
constexpr std::uint32_t kPausedDelayMs = 50;

}

Application::Application(ApplicationConfig config) : config_(std::move(config)) {}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    SDL_SetAppMetadata(config_.name.c_str(), config_.version.c_str(), config_.identifier.c_str());
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown");

    if (!SDL_Init(SDL_INIT_VIDEO)) return false;
    sdlReady_ = true;

    WindowConfig windowConfig = config_.window;
    if (windowConfig.title.empty()) windowConfig.title = config_.name;
    if (!window_.Create(windowConfig)) return false;
    if (!renderer_.Create(window_, config_.vsync)) return false;

    if (config_.useDefaultIcon) {
        const IconImage icon = DefaultIcon();
        window_.SetIcon(icon.pixels, icon.width, icon.height);
    }
    return true;
}

void Application::Shutdown() {
    if (scene_) {
        scene_->OnExit(*this);
        scene_.reset();
    }
    pendingScene_.reset();
    renderer_.Destroy();
    window_.Destroy();
    if (sdlReady_) {
        SDL_Quit();
        sdlReady_ = false;
    }
}

void Application::Quit() {
    running_ = false;
}

void Application::ApplyPendingScene() {
    if (!pendingScene_) return;
    if (scene_) scene_->OnExit(*this);
    scene_ = std::move(pendingScene_);
    scene_->OnEnter(*this);
}

void Application::PumpEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                OnEvent(event);
                if (OnQuitRequested()) running_ = false;
                break;
            case SDL_EVENT_TERMINATING:
                running_ = false;
                break;
            case SDL_EVENT_WILL_ENTER_BACKGROUND:
                if (!paused_) {
                    paused_ = true;
                    OnPause();
                }
                break;
            case SDL_EVENT_DID_ENTER_FOREGROUND:
                if (paused_) {
                    paused_ = false;
                    OnResume();
                }
                break;
            case SDL_EVENT_LOW_MEMORY:
                OnLowMemory();
                break;
            default:
                OnEvent(event);
                input_.HandleEvent(event, renderer_);
                break;
        }
    }
}

void Application::Step(float dt) {
    accumulator_ += dt;
    const double fixed = static_cast<double>(config_.fixedStep);
    while (fixed > 0.0 && accumulator_ >= fixed) {
        OnFixedUpdate(config_.fixedStep);
        accumulator_ -= fixed;
    }
    OnUpdate(dt);
    if (scene_) scene_->OnUpdate(*this, dt);
}

void Application::Draw() {
    renderer_.BeginFrame(config_.clearColor);
    if (scene_) scene_->OnRender(*this, renderer_);
    OnRender(renderer_);
    renderer_.EndFrame();
}

int Application::Run() {
    if (!Initialize()) {
        Shutdown();
        return 1;
    }

    running_ = true;
    OnStart();

    std::uint64_t last = SDL_GetTicksNS();
    while (running_) {
        input_.BeginFrame();
        PumpEvents();
        if (!running_) break;

        const std::uint64_t now = SDL_GetTicksNS();
        double dt = static_cast<double>(now - last) / 1.0e9;
        last = now;
        if (dt > kMaxFrameTime) dt = kMaxFrameTime;

        if (paused_) {
            SDL_Delay(kPausedDelayMs);
            continue;
        }

        ApplyPendingScene();
        Step(static_cast<float>(dt));
        Draw();

        totalTime_ += dt;
        ++frame_;
        if (config_.maxFrames != 0 && frame_ >= config_.maxFrames) running_ = false;
    }

    OnShutdown();
    Shutdown();
    return 0;
}

}
