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
            std::string TextureName = "";
            std::string ShaderName  = "base";
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
