#include "core/Icon.h"

#include "EmbeddedIcon.h"

namespace core {

IconImage DefaultIcon() {
    return {detail::kEmbeddedIconPixels, detail::kEmbeddedIconSize, detail::kEmbeddedIconSize};
}

}
