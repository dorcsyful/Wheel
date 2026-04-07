#pragma once
#include <cstdint>

namespace Wheel
{
    namespace Components
    {
        struct Render2DComponent
        {
            bool active;
            uint32_t ID;
            size_t TextureName;
            float top;
            float left;
            float width;
            float height;
        };
    }
}
