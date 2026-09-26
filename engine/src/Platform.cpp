#include "core/Platform.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace core {

Platform CurrentPlatform() {
#if defined(__ANDROID__)
    return Platform::Android;
#elif defined(__APPLE__) && TARGET_OS_IPHONE
    return Platform::IOS;
#elif defined(__APPLE__)
    return Platform::MacOS;
#elif defined(_WIN32)
    return Platform::Windows;
#elif defined(__linux__)
    return Platform::Linux;
#else
    return Platform::Unknown;
#endif
}

const char* PlatformName() {
    switch (CurrentPlatform()) {
        case Platform::Windows: return "Windows";
        case Platform::Linux: return "Linux";
        case Platform::MacOS: return "macOS";
        case Platform::IOS: return "iOS";
        case Platform::Android: return "Android";
        default: return "Unknown";
    }
}

bool IsMobile() {
    const Platform platform = CurrentPlatform();
    return platform == Platform::IOS || platform == Platform::Android;
}

}
