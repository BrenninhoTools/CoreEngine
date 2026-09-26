#include "core/Renderer.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <vector>

#include "core/Window.h"

namespace core {

namespace {

constexpr int kCircleSegments = 48;
constexpr float kPi = 3.14159265358979323846f;

SDL_FRect ToSdl(const Rect& r) {
    return SDL_FRect{r.x, r.y, r.w, r.h};
}

}

Renderer::~Renderer() {
    Destroy();
}

bool Renderer::Create(Window& window, bool vsync) {
    Destroy();
    handle_ = SDL_CreateRenderer(window.Handle(), nullptr);
    if (handle_ == nullptr) return false;
    SDL_SetRenderVSync(handle_, vsync ? 1 : 0);
    SDL_SetRenderDrawBlendMode(handle_, SDL_BLENDMODE_BLEND);
    return true;
}

void Renderer::Destroy() {
    if (handle_ != nullptr) {
        SDL_DestroyRenderer(handle_);
        handle_ = nullptr;
    }
}

void Renderer::ApplyColor(Color color) {
    SDL_SetRenderDrawColor(handle_, color.r, color.g, color.b, color.a);
}

void Renderer::BeginFrame(Color clear) {
    ApplyColor(clear);
    SDL_RenderClear(handle_);
}

void Renderer::EndFrame() {
    SDL_RenderPresent(handle_);
}

void Renderer::FillRect(const Rect& rect, Color color) {
    ApplyColor(color);
    const SDL_FRect r = ToSdl(rect);
    SDL_RenderFillRect(handle_, &r);
}

void Renderer::DrawRect(const Rect& rect, Color color) {
    ApplyColor(color);
    const SDL_FRect r = ToSdl(rect);
    SDL_RenderRect(handle_, &r);
}

void Renderer::DrawLine(Vec2 from, Vec2 to, Color color) {
    ApplyColor(color);
    SDL_RenderLine(handle_, from.x, from.y, to.x, to.y);
}

void Renderer::FillCircle(Vec2 center, float radius, Color color) {
    const SDL_FColor c{
        static_cast<float>(color.r) / 255.0f,
        static_cast<float>(color.g) / 255.0f,
        static_cast<float>(color.b) / 255.0f,
        static_cast<float>(color.a) / 255.0f};

    std::vector<SDL_Vertex> vertices(static_cast<std::size_t>(kCircleSegments) + 1);
    std::vector<int> indices;
    indices.reserve(static_cast<std::size_t>(kCircleSegments) * 3);

    vertices[0] = SDL_Vertex{SDL_FPoint{center.x, center.y}, c, SDL_FPoint{0.0f, 0.0f}};
    for (int i = 0; i < kCircleSegments; ++i) {
        const float angle = (static_cast<float>(i) / static_cast<float>(kCircleSegments)) * 2.0f * kPi;
        vertices[static_cast<std::size_t>(i) + 1] = SDL_Vertex{
            SDL_FPoint{center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius},
            c,
            SDL_FPoint{0.0f, 0.0f}};
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back((i + 1) % kCircleSegments + 1);
    }

    SDL_RenderGeometry(
        handle_, nullptr, vertices.data(), static_cast<int>(vertices.size()),
        indices.data(), static_cast<int>(indices.size()));
}

void Renderer::DrawText(Vec2 position, const std::string& text, Color color, float scale) {
    ApplyColor(color);
    SDL_SetRenderScale(handle_, scale, scale);
    SDL_RenderDebugText(handle_, position.x / scale, position.y / scale, text.c_str());
    SDL_SetRenderScale(handle_, 1.0f, 1.0f);
}

Vec2 Renderer::OutputSize() const {
    int w = 0;
    int h = 0;
    if (handle_ != nullptr) SDL_GetCurrentRenderOutputSize(handle_, &w, &h);
    return {static_cast<float>(w), static_cast<float>(h)};
}

}
