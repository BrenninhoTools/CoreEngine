#include "core/Scene.h"

#include <algorithm>

#include "core/Renderer.h"

namespace core {

void Scene::OnUpdate(Application& app, float dt) {
    (void)app;
    for (std::size_t i = 0; i < entities_.size(); ++i) {
        entities_[i]->OnUpdate(dt);
    }
    entities_.erase(
        std::remove_if(entities_.begin(), entities_.end(),
                       [](const std::unique_ptr<Entity>& e) { return !e->Alive(); }),
        entities_.end());
}

void Scene::OnRender(Application& app, Renderer& renderer) {
    (void)app;
    for (const auto& entity : entities_) {
        entity->OnRender(renderer);
    }
}

}
