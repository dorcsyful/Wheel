#pragma once
#include <cstdint>

#include "DebugDescriptor.h"

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

            REFLECT_BEGIN(Render2DComponent)
            FIELD(active)
            FIELD(ID)
            FIELD(TextureName)
            FIELD(width)
            FIELD(height)
            REFLECT_END(Render2DComponent, "Render2DComponent")
        };
    }
}
