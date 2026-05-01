#pragma once
#include <cstdint>

namespace Wheel
{
    namespace Components
    {
        struct Render2DComponent
        {
            bool active = true;
            uint32_t ID = 999999;
            size_t TextureName = 0;
            /**
             * @brief In world units
             */
            float width = 0;
            /**
             * @brief In world units
             */
            float height = 0;
        };
    }
}
