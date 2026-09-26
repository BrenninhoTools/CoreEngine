#pragma once

#include <cstdint>
#include <string>

#include "core/Math.h"

struct SDL_Window;

namespace core {

struct WindowConfig {
    std::string title = "Core Engine";
    int width = 1280;
    int height = 720;
    bool resizable = true;
    bool fullscreen = false;
    bool highDensity = true;
};

class Window {
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Create(const WindowConfig& config);
    void Destroy();

    void SetTitle(const std::string& title);
    void SetIcon(const std::uint8_t* rgba, int width, int height);

    Vec2 PixelSize() const;
    Rect SafeArea() const;

    SDL_Window* Handle() const { return handle_; }

private:
    SDL_Window* handle_ = nullptr;
};

}
