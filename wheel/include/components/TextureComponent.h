#pragma once
#include "math/Vector4.h"

namespace Wheel
{
    namespace Components
    {
        struct TextureComponent
        {
            /**
             * @brief The ID of the texture to render. You can query it via Renderer->GetTextureID(filename)
             */
            unsigned int textureID;
            /**
             * @brief: You can give the texture a tint. If there is no texture assigned, this will be the color of the mesh
             */
            Math::Vector4 color;
        };
    }
}