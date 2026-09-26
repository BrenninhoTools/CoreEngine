#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace core {

class Application;
class Renderer;

class Entity {
public:
    virtual ~Entity() = default;

    virtual void OnUpdate(float dt) { (void)dt; }
    virtual void OnRender(Renderer& renderer) { (void)renderer; }

    void Destroy() { alive_ = false; }
    bool Alive() const { return alive_; }

private:
    bool alive_ = true;
};

class Scene {
public:
    virtual ~Scene() = default;

    virtual void OnEnter(Application& app) { (void)app; }
    virtual void OnExit(Application& app) { (void)app; }
    virtual void OnUpdate(Application& app, float dt);
    virtual void OnRender(Application& app, Renderer& renderer);

    template <typename T, typename... Args>
    T& Spawn(Args&&... args) {
        static_assert(std::is_base_of_v<Entity, T>, "T must derive from core::Entity");
        auto entity = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *entity;
        entities_.push_back(std::move(entity));
        return ref;
    }

    std::size_t EntityCount() const { return entities_.size(); }

private:
    std::vector<std::unique_ptr<Entity>> entities_;
};

}
