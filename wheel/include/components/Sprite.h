#pragma once
#include <cstdint>

#include "DebugDescriptor.h"
#include "systems/helpers/RenderLayers.h"

namespace Wheel
{
    namespace Components
    {
        struct Sprite
        {
            bool active = true;
            uint32_t ID = 999999;
            uint32_t MaterialName = 0;
            /**
             * @brief In world units
             */
            float width = 0;
            /**
             * @brief In world units
             */
            float height = 0;
            /**
             * @brief: You can give the texture a tint. If there is no texture assigned, this will be the color of the mesh
             */
            Math::Vector4 color = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            Helpers::RenderLevel level = 0;
            REFLECT_BEGIN(Sprite)
            FIELD(active)
            FIELD(ID)
            FIELD(MaterialName)
            FIELD(width)
            FIELD(height)
            REFLECT_END(Sprite, "Sprite")
        };
    }
}
