#pragma once

namespace core {

enum class Platform {
    Windows,
    Linux,
    MacOS,
    IOS,
    Android,
    Unknown
};

Platform CurrentPlatform();
const char* PlatformName();
bool IsMobile();

}
