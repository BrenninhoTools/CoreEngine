#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>

#include "core/Application.h"
#include "core/EntryPoint.h"
#include "core/Platform.h"

namespace {

class Ball : public core::Entity {
public:
    Ball(core::Vec2 position, core::Vec2 velocity, float radius, core::Color color, const core::Rect& bounds)
        : position_(position), velocity_(velocity), radius_(radius), color_(color), bounds_(bounds) {}

    void OnUpdate(float dt) override {
        position_ += velocity_ * dt;
        if (position_.x - radius_ < bounds_.x) {
            position_.x = bounds_.x + radius_;
            velocity_.x = std::fabs(velocity_.x);
        } else if (position_.x + radius_ > bounds_.x + bounds_.w) {
            position_.x = bounds_.x + bounds_.w - radius_;
            velocity_.x = -std::fabs(velocity_.x);
        }
        if (position_.y - radius_ < bounds_.y) {
            position_.y = bounds_.y + radius_;
            velocity_.y = std::fabs(velocity_.y);
        } else if (position_.y + radius_ > bounds_.y + bounds_.h) {
            position_.y = bounds_.y + bounds_.h - radius_;
            velocity_.y = -std::fabs(velocity_.y);
        }
    }

    void OnRender(core::Renderer& renderer) override {
        renderer.FillCircle(position_, radius_, color_);
    }

private:
    core::Vec2 position_;
    core::Vec2 velocity_;
    float radius_;
    core::Color color_;
    const core::Rect& bounds_;
};

class Player : public core::Entity {
public:
    core::Vec2 position;
    float size = 48.0f;

    void OnRender(core::Renderer& renderer) override {
        const core::Rect body{position.x - size * 0.5f, position.y - size * 0.5f, size, size};
        renderer.FillRect(body, {56, 189, 248, 255});
        renderer.DrawRect(body, {236, 250, 255, 255});
    }
};

class SandboxScene : public core::Scene {
public:
    void OnEnter(core::Application& app) override {
        playfield_ = app.GetWindow().SafeArea();
        player_ = &Spawn<Player>();
        player_->position = playfield_.Center();

        const core::Color palette[] = {
            {250, 204, 21, 255}, {248, 113, 113, 255}, {74, 222, 128, 255}, {192, 132, 252, 255}};
        for (int i = 0; i < 8; ++i) {
            const float t = static_cast<float>(i);
            const core::Vec2 position{playfield_.x + playfield_.w * (0.15f + 0.1f * t),
                                      playfield_.y + playfield_.h * (0.2f + 0.08f * t)};
            const core::Vec2 velocity{(t - 3.5f) * 60.0f + 90.0f, 140.0f - t * 30.0f};
            Spawn<Ball>(position, velocity, 14.0f + t * 2.0f, palette[i % 4], playfield_);
        }
    }

    void OnUpdate(core::Application& app, float dt) override {
        playfield_ = app.GetWindow().SafeArea();
        const core::Input& input = app.GetInput();

        core::Vec2 direction;
        if (input.KeyDown(SDL_SCANCODE_LEFT) || input.KeyDown(SDL_SCANCODE_A)) direction.x -= 1.0f;
        if (input.KeyDown(SDL_SCANCODE_RIGHT) || input.KeyDown(SDL_SCANCODE_D)) direction.x += 1.0f;
        if (input.KeyDown(SDL_SCANCODE_UP) || input.KeyDown(SDL_SCANCODE_W)) direction.y -= 1.0f;
        if (input.KeyDown(SDL_SCANCODE_DOWN) || input.KeyDown(SDL_SCANCODE_S)) direction.y += 1.0f;

        if (input.PointerDown()) {
            const core::Vec2 toPointer = input.PointerPosition() - player_->position;
            if (toPointer.Length() > 8.0f) direction = toPointer;
        }

        const float speed = std::min(playfield_.w, playfield_.h) * 0.7f;
        player_->position += direction.Normalized() * speed * dt;

        const float half = player_->size * 0.5f;
        player_->position.x = core::Clamp(player_->position.x, playfield_.x + half, playfield_.x + playfield_.w - half);
        player_->position.y = core::Clamp(player_->position.y, playfield_.y + half, playfield_.y + playfield_.h - half);

        if (dt > 0.0f) fps_ += (1.0f / dt - fps_) * 0.1f;

        core::Scene::OnUpdate(app, dt);
    }

    void OnRender(core::Application& app, core::Renderer& renderer) override {
        const core::Vec2 size = renderer.OutputSize();
        const core::Color grid{255, 255, 255, 18};
        const float step = 64.0f;
        for (float x = 0.0f; x < size.x; x += step) renderer.DrawLine({x, 0.0f}, {x, size.y}, grid);
        for (float y = 0.0f; y < size.y; y += step) renderer.DrawLine({0.0f, y}, {size.x, y}, grid);
        renderer.DrawRect(playfield_, {56, 189, 248, 90});

        core::Scene::OnRender(app, renderer);

        const float scale = std::max(1.0f, std::floor(size.y / 360.0f));
        const std::string hud = std::string("Core Engine  ") + core::PlatformName() + "  " +
                                std::to_string(static_cast<int>(fps_ + 0.5f)) + " FPS";
        renderer.DrawText({playfield_.x + 12.0f * scale, playfield_.y + 12.0f * scale}, hud,
                          {236, 250, 255, 255}, scale);
    }

private:
    core::Rect playfield_;
    Player* player_ = nullptr;
    float fps_ = 60.0f;
};

class SandboxApp : public core::Application {
public:
    using core::Application::Application;

protected:
    void OnUpdate(float dt) override {
        (void)dt;
        const core::Input& input = GetInput();
        if (input.KeyPressed(SDL_SCANCODE_ESCAPE) || input.KeyPressed(SDL_SCANCODE_AC_BACK)) Quit();
    }
};

}

int main(int argc, char** argv) {
    core::ApplicationConfig config;
    config.name = "Core Engine Sandbox";
    config.identifier = "com.coreengine.sandbox";
    config.window.title = "Core Engine Sandbox";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            config.maxFrames = std::strtoull(argv[++i], nullptr, 10);
        }
    }

    SandboxApp app(config);
    app.SetScene<SandboxScene>();
    return app.Run();
}
