#pragma once
#include <cmath>
#include <cstdint>

#include "components/Render2DComponent.h"
#include "components/Transform2D.h"
#include "math/Matrix4x4.h"

namespace Wheel
{
    namespace Renderer
    {
        /**
         * @brief The connecting object between the RenderSystem and the actual renderer
         */
        class RenderedObject
        {
        public:
            void Add2DComponent(const Components::Render2DComponent& a_Render, uint32_t a_EntityId, const Components::Transform2D& a_Transform)
            {
                entityId = a_EntityId;
                textureId = a_Render.TextureName;
                //build the matrix:
                float cos_r = std::cos(a_Transform.rotation);
                float sin_r = std::sin(a_Transform.rotation);
                auto position = Math::Vector2(a_Transform.position.x, a_Transform.position.y);
                auto scale = Math::Vector2(a_Render.width * a_Transform.scale.x, a_Render.height * a_Transform.scale.y);
                modelMatrix = Math::Matrix4x4(scale.x * cos_r, -scale.y * sin_r, 0.0, position.x,
                                              scale.x * sin_r, scale.y * cos_r, 0.0, position.y,
                                              0.0, 0.0, 1.0, 0.0,
                                              0.0, 0.0, 0.0, 1.0);
            }
            size_t entityId;
            size_t textureId;
            size_t shaderId;
            Math::Matrix4x4 modelMatrix;
        };
    }
}
