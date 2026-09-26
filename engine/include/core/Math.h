#pragma once

#include <cmath>
#include <cstdint>

namespace core {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float px, float py) : x(px), y(py) {}

    constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }
    constexpr Vec2 operator-() const { return {-x, -y}; }
    Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(Vec2 o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    float Length() const { return std::sqrt(x * x + y * y); }
    float Dot(Vec2 o) const { return x * o.x + y * o.y; }

    Vec2 Normalized() const {
        const float len = Length();
        return len > 0.0f ? Vec2{x / len, y / len} : Vec2{};
    }
};

constexpr Vec2 operator*(float s, Vec2 v) { return v * s; }

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    constexpr bool Contains(Vec2 p) const {
        return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h;
    }

    constexpr Vec2 Center() const { return {x + w * 0.5f, y + h * 0.5f}; }
};

struct Color {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    static constexpr Color White() { return {255, 255, 255, 255}; }
    static constexpr Color Black() { return {0, 0, 0, 255}; }
    static constexpr Color Transparent() { return {0, 0, 0, 0}; }
};

template <typename T>
constexpr T Clamp(T value, T low, T high) {
    return value < low ? low : (value > high ? high : value);
}

}
