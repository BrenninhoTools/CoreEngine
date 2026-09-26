#pragma once

#include <string>
#include <vector>

#include "core/Math.h"
#include "core/Renderer.h"

namespace core {

enum class Shape {
    Rect,
    Circle
};

struct EntityData {
    std::string name = "Entity";
    Shape shape = Shape::Rect;
    Vec2 position;
    Vec2 size{64.0f, 64.0f};
    Color color{56, 189, 248, 255};
    Vec2 velocity;

    Rect Bounds() const;
    bool Contains(Vec2 point) const;
};

struct SceneData {
    std::string name = "Untitled";
    Color background{16, 18, 28, 255};
    std::vector<EntityData> entities;

    bool Save(const std::string& path) const;
    static bool Load(const std::string& path, SceneData& out);

    void Step(float dt, const Rect& area);
    void Draw(Renderer& renderer) const;
    int Pick(Vec2 point) const;
};

}
