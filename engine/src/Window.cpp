#include "core/Window.h"

#include <SDL3/SDL.h>

#include "core/Platform.h"

namespace core {

Window::~Window() {
    Destroy();
}

bool Window::Create(const WindowConfig& config) {
    Destroy();

    SDL_WindowFlags flags = 0;
    if (config.resizable) flags |= SDL_WINDOW_RESIZABLE;
    if (config.highDensity) flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (config.fullscreen || IsMobile()) flags |= SDL_WINDOW_FULLSCREEN;

    handle_ = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    return handle_ != nullptr;
}

void Window::Destroy() {
    if (handle_ != nullptr) {
        SDL_DestroyWindow(handle_);
        handle_ = nullptr;
    }
}

void Window::SetTitle(const std::string& title) {
    if (handle_ != nullptr) SDL_SetWindowTitle(handle_, title.c_str());
}

void Window::SetIcon(const std::uint8_t* rgba, int width, int height) {
    if (handle_ == nullptr) return;
    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        width, height, SDL_PIXELFORMAT_RGBA32, const_cast<std::uint8_t*>(rgba), width * 4);
    if (surface == nullptr) return;
    SDL_SetWindowIcon(handle_, surface);
    SDL_DestroySurface(surface);
}

Vec2 Window::PixelSize() const {
    int w = 0;
    int h = 0;
    if (handle_ != nullptr) SDL_GetWindowSizeInPixels(handle_, &w, &h);
    return {static_cast<float>(w), static_cast<float>(h)};
}

Rect Window::SafeArea() const {
    int pw = 0;
    int ph = 0;
    int ww = 0;
    int wh = 0;
    SDL_Rect area{};
    if (handle_ == nullptr) return {};
    SDL_GetWindowSizeInPixels(handle_, &pw, &ph);
    SDL_GetWindowSize(handle_, &ww, &wh);
    if (!SDL_GetWindowSafeArea(handle_, &area) || ww <= 0 || wh <= 0) {
        return {0.0f, 0.0f, static_cast<float>(pw), static_cast<float>(ph)};
    }
    const float sx = static_cast<float>(pw) / static_cast<float>(ww);
    const float sy = static_cast<float>(ph) / static_cast<float>(wh);
    return {
        static_cast<float>(area.x) * sx,
        static_cast<float>(area.y) * sy,
        static_cast<float>(area.w) * sx,
        static_cast<float>(area.h) * sy};
}

}
