#pragma once

#include <string>

#include "core/Math.h"

struct SDL_Renderer;

namespace core {

class Window;

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool Create(Window& window, bool vsync);
    void Destroy();

    void BeginFrame(Color clear);
    void EndFrame();

    void FillRect(const Rect& rect, Color color);
    void DrawRect(const Rect& rect, Color color);
    void DrawLine(Vec2 from, Vec2 to, Color color);
    void FillCircle(Vec2 center, float radius, Color color);
    void DrawText(Vec2 position, const std::string& text, Color color, float scale = 1.0f);

    Vec2 OutputSize() const;

    SDL_Renderer* Handle() const { return handle_; }

private:
    void ApplyColor(Color color);

    SDL_Renderer* handle_ = nullptr;
};

}
