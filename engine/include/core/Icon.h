#pragma once

#include <cstdint>

namespace core {

struct IconImage {
    const std::uint8_t* pixels = nullptr;
    int width = 0;
    int height = 0;
};

IconImage DefaultIcon();

}
