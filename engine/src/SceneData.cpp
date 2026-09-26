#include "core/SceneData.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace core {

namespace {

constexpr const char* kHeader = "core-scene 1";

std::string Sanitize(const std::string& value) {
    std::string out = value;
    for (char& c : out) {
        if (c == '\n' || c == '\r') c = ' ';
    }
    return out;
}

Color ReadColor(std::istringstream& in) {
    int r = 255;
    int g = 255;
    int b = 255;
    int a = 255;
    in >> r >> g >> b >> a;
    return {
        static_cast<std::uint8_t>(Clamp(r, 0, 255)),
        static_cast<std::uint8_t>(Clamp(g, 0, 255)),
        static_cast<std::uint8_t>(Clamp(b, 0, 255)),
        static_cast<std::uint8_t>(Clamp(a, 0, 255))};
}

Vec2 ReadVec2(std::istringstream& in) {
    Vec2 v;
    in >> v.x >> v.y;
    return v;
}

std::string Trim(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return {};
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

}

Rect EntityData::Bounds() const {
    return {position.x - size.x * 0.5f, position.y - size.y * 0.5f, size.x, size.y};
}

bool EntityData::Contains(Vec2 point) const {
    if (shape == Shape::Circle) {
        const float radius = size.x * 0.5f;
        return (point - position).Length() <= radius;
    }
    return Bounds().Contains(point);
}

bool SceneData::Save(const std::string& path) const {
    std::ofstream file(std::filesystem::u8path(path), std::ios::binary | std::ios::trunc);
    if (!file) return false;

    file << kHeader << '\n';
    file << "name " << Sanitize(name) << '\n';
    file << "background " << int(background.r) << ' ' << int(background.g) << ' '
         << int(background.b) << ' ' << int(background.a) << '\n';

    for (const EntityData& e : entities) {
        file << "entity " << Sanitize(e.name) << '\n';
        file << "shape " << (e.shape == Shape::Circle ? "circle" : "rect") << '\n';
        file << "position " << e.position.x << ' ' << e.position.y << '\n';
        file << "size " << e.size.x << ' ' << e.size.y << '\n';
        file << "color " << int(e.color.r) << ' ' << int(e.color.g) << ' '
             << int(e.color.b) << ' ' << int(e.color.a) << '\n';
        file << "velocity " << e.velocity.x << ' ' << e.velocity.y << '\n';
        file << "end\n";
    }
    return static_cast<bool>(file);
}

bool SceneData::Load(const std::string& path, SceneData& out) {
    std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
    if (!file) return false;

    std::string line;
    if (!std::getline(file, line) || Trim(line) != kHeader) return false;

    SceneData scene;
    EntityData* current = nullptr;

    while (std::getline(file, line)) {
        std::istringstream in(line);
        std::string key;
        in >> key;
        if (key.empty()) continue;

        std::string rest;
        std::getline(in, rest);
        rest = Trim(rest);
        std::istringstream args(rest);

        if (key == "name" && current == nullptr) {
            scene.name = rest;
        } else if (key == "background" && current == nullptr) {
            scene.background = ReadColor(args);
        } else if (key == "entity") {
            scene.entities.emplace_back();
            current = &scene.entities.back();
            current->name = rest;
        } else if (key == "end") {
            current = nullptr;
        } else if (current != nullptr) {
            if (key == "shape") current->shape = rest == "circle" ? Shape::Circle : Shape::Rect;
            else if (key == "position") current->position = ReadVec2(args);
            else if (key == "size") current->size = ReadVec2(args);
            else if (key == "color") current->color = ReadColor(args);
            else if (key == "velocity") current->velocity = ReadVec2(args);
        }
    }

    out = std::move(scene);
    return true;
}

void SceneData::Step(float dt, const Rect& area) {
    for (EntityData& e : entities) {
        if (e.velocity.x == 0.0f && e.velocity.y == 0.0f) continue;
        e.position += e.velocity * dt;

        const float hx = e.size.x * 0.5f;
        const float hy = e.size.y * 0.5f;
        if (e.position.x - hx < area.x) {
            e.position.x = area.x + hx;
            e.velocity.x = std::fabs(e.velocity.x);
        } else if (e.position.x + hx > area.x + area.w) {
            e.position.x = area.x + area.w - hx;
            e.velocity.x = -std::fabs(e.velocity.x);
        }
        if (e.position.y - hy < area.y) {
            e.position.y = area.y + hy;
            e.velocity.y = std::fabs(e.velocity.y);
        } else if (e.position.y + hy > area.y + area.h) {
            e.position.y = area.y + area.h - hy;
            e.velocity.y = -std::fabs(e.velocity.y);
        }
    }
}

void SceneData::Draw(Renderer& renderer) const {
    for (const EntityData& e : entities) {
        if (e.shape == Shape::Circle) {
            renderer.FillCircle(e.position, e.size.x * 0.5f, e.color);
        } else {
            renderer.FillRect(e.Bounds(), e.color);
        }
    }
}

int SceneData::Pick(Vec2 point) const {
    for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i) {
        if (entities[static_cast<std::size_t>(i)].Contains(point)) return i;
    }
    return -1;
}

}
