#pragma once

#include <SDL3/SDL_stdinc.h>

#include <mutex>
#include <string>
#include <vector>

#include "Project.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace core {
class Window;
}

namespace editor {

struct HomeRequest {
    enum class Kind {
        None,
        Open
    };

    Kind kind = Kind::None;
    std::string root;
};

class HomeScreen {
public:
    HomeScreen(core::Window& window, SDL_Renderer* renderer, RecentProjects& recent);
    ~HomeScreen();

    HomeScreen(const HomeScreen&) = delete;
    HomeScreen& operator=(const HomeScreen&) = delete;

    HomeRequest Draw();
    void SetError(const std::string& message);
    void Refresh();

private:
    struct DialogResult {
        std::mutex mutex;
        std::string value;
        bool ready = false;
    };

    static void SDLCALL OnFolderChosen(void* userdata, const char* const* files, int filter);
    bool TakeResult(DialogResult& result, std::string& value);
    void DrawHeader(float unit);
    void DrawRecent(HomeRequest& request, float unit);
    void DrawCreate(HomeRequest& request, float unit);

    core::Window& window_;
    SDL_Renderer* renderer_;
    RecentProjects& recent_;
    SDL_Texture* logo_ = nullptr;

    std::vector<bool> missing_;
    char name_[128] = "MyGame";
    char location_[1024] = {};
    int templateIndex_ = 1;
    std::string error_;

    DialogResult openResult_;
    DialogResult browseResult_;
};

}
