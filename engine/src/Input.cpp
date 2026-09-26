#include "core/Input.h"

#include <SDL3/SDL.h>

#include <algorithm>

#include "core/Renderer.h"

namespace core {

namespace {

bool InRange(int index, int count) {
    return index >= 0 && index < count;
}

}

void Input::BeginFrame() {
    keyPressed_.fill(false);
    keyReleased_.fill(false);
    mousePressed_.fill(false);
    mouseReleased_.fill(false);
    pointerPressed_ = false;
    pointerReleased_ = false;
}

void Input::HandleEvent(const SDL_Event& event, const Renderer& renderer) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN: {
            const int key = static_cast<int>(event.key.scancode);
            if (!InRange(key, kKeyCount)) break;
            if (!event.key.repeat) keyPressed_[key] = true;
            keyDown_[key] = true;
            break;
        }
        case SDL_EVENT_KEY_UP: {
            const int key = static_cast<int>(event.key.scancode);
            if (!InRange(key, kKeyCount)) break;
            keyDown_[key] = false;
            keyReleased_[key] = true;
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Event converted = event;
            SDL_ConvertEventToRenderCoordinates(renderer.Handle(), &converted);
            mousePosition_ = {converted.motion.x, converted.motion.y};
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            SDL_Event converted = event;
            SDL_ConvertEventToRenderCoordinates(renderer.Handle(), &converted);
            mousePosition_ = {converted.button.x, converted.button.y};
            const int button = event.button.button;
            if (!InRange(button, kMouseButtonCount)) break;
            const bool down = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
            mouseDown_[button] = down;
            (down ? mousePressed_ : mouseReleased_)[button] = true;
            if (button == static_cast<int>(MouseButton::Left)) {
                (down ? pointerPressed_ : pointerReleased_) = true;
            }
            break;
        }
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_MOTION: {
            const Vec2 size = renderer.OutputSize();
            const Vec2 position{event.tfinger.x * size.x, event.tfinger.y * size.y};
            const std::uint64_t id = static_cast<std::uint64_t>(event.tfinger.fingerID);
            auto it = std::find_if(touches_.begin(), touches_.end(),
                                   [id](const Touch& t) { return t.id == id; });
            if (it == touches_.end()) {
                touches_.push_back({id, position});
                if (touches_.size() == 1) pointerPressed_ = true;
            } else {
                it->position = position;
            }
            break;
        }
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_CANCELED: {
            const std::uint64_t id = static_cast<std::uint64_t>(event.tfinger.fingerID);
            auto it = std::find_if(touches_.begin(), touches_.end(),
                                   [id](const Touch& t) { return t.id == id; });
            if (it != touches_.end()) {
                touches_.erase(it);
                if (touches_.empty()) pointerReleased_ = true;
            }
            break;
        }
        default:
            break;
    }
}

bool Input::KeyDown(Key key) const {
    const int i = static_cast<int>(key);
    return InRange(i, kKeyCount) && keyDown_[i];
}

bool Input::KeyPressed(Key key) const {
    const int i = static_cast<int>(key);
    return InRange(i, kKeyCount) && keyPressed_[i];
}

bool Input::KeyReleased(Key key) const {
    const int i = static_cast<int>(key);
    return InRange(i, kKeyCount) && keyReleased_[i];
}

bool Input::MouseDown(MouseButton button) const {
    const int i = static_cast<int>(button);
    return InRange(i, kMouseButtonCount) && mouseDown_[i];
}

bool Input::MousePressed(MouseButton button) const {
    const int i = static_cast<int>(button);
    return InRange(i, kMouseButtonCount) && mousePressed_[i];
}

bool Input::MouseReleased(MouseButton button) const {
    const int i = static_cast<int>(button);
    return InRange(i, kMouseButtonCount) && mouseReleased_[i];
}

bool Input::PointerDown() const {
    return !touches_.empty() || MouseDown(MouseButton::Left);
}

Vec2 Input::PointerPosition() const {
    return touches_.empty() ? mousePosition_ : touches_.front().position;
}

}
