#pragma once

#include <SDL3/SDL_scancode.h>

#include <array>
#include <cstdint>
#include <vector>

#include "core/Math.h"

union SDL_Event;

namespace core {

class Renderer;

using Key = SDL_Scancode;

enum class MouseButton : int {
    Left = 1,
    Middle = 2,
    Right = 3
};

struct Touch {
    std::uint64_t id = 0;
    Vec2 position;
};

class Input {
public:
    void BeginFrame();
    void HandleEvent(const SDL_Event& event, const Renderer& renderer);

    bool KeyDown(Key key) const;
    bool KeyPressed(Key key) const;
    bool KeyReleased(Key key) const;

    Vec2 MousePosition() const { return mousePosition_; }
    bool MouseDown(MouseButton button) const;
    bool MousePressed(MouseButton button) const;
    bool MouseReleased(MouseButton button) const;

    const std::vector<Touch>& Touches() const { return touches_; }

    bool PointerDown() const;
    bool PointerPressed() const { return pointerPressed_; }
    bool PointerReleased() const { return pointerReleased_; }
    Vec2 PointerPosition() const;

private:
    static constexpr int kKeyCount = SDL_SCANCODE_COUNT;
    static constexpr int kMouseButtonCount = 8;

    std::array<bool, kKeyCount> keyDown_{};
    std::array<bool, kKeyCount> keyPressed_{};
    std::array<bool, kKeyCount> keyReleased_{};

    std::array<bool, kMouseButtonCount> mouseDown_{};
    std::array<bool, kMouseButtonCount> mousePressed_{};
    std::array<bool, kMouseButtonCount> mouseReleased_{};
    Vec2 mousePosition_;

    std::vector<Touch> touches_;
    bool pointerPressed_ = false;
    bool pointerReleased_ = false;
};

}
