#pragma once
#include <cmath>
#include <cstdint>
#include "../../wheel/include/core/Globals.h"
#include "components/CameraComponent.h"
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
            void Add2DComponent(const Components::Render2DComponent& a_Render, uint32_t a_EntityId,
                                const Components::Transform2D& a_Transform,
                                const Components::Transform2D& a_CameraTransform,
                                const Components::CameraComponent& a_Camera)
            {
                entityId = a_EntityId;
                textureId = a_Render.TextureName;

                // Model matrix: scale by (width*scaleX, height*scaleY), rotate, translate
                constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
                float cos_r = std::cos(a_Transform.GetRotation() * DEG_TO_RAD);
                float sin_r = std::sin(a_Transform.GetRotation() * DEG_TO_RAD);
                float sx = a_Render.width  * a_Transform.GetScale().x;
                float sy = a_Render.height * a_Transform.GetScale().y;
                float px = a_Transform.GetPosition().x;
                float py = a_Transform.GetPosition().y;

                Math::Matrix4x4 model(
                    sx * cos_r, -sy * sin_r, 0.0f, px,
                    sx * sin_r,  sy * cos_r, 0.0f, py,
                    0.0f,        0.0f,       1.0f, 0.0f,
                    0.0f,        0.0f,       0.0f, 1.0f
                );

                // View matrix: inverse of camera's world transform (translate then rotate)
                float cos_cr = std::cos(a_CameraTransform.GetRotation() * DEG_TO_RAD);
                float sin_cr = std::sin(a_CameraTransform.GetRotation() * DEG_TO_RAD);
                float cx = a_CameraTransform.GetPosition().x;
                float cy = a_CameraTransform.GetPosition().y;

                Math::Matrix4x4 view(
                     cos_cr,  sin_cr, 0.0f, -(cx * cos_cr + cy * sin_cr),
                    -sin_cr,  cos_cr, 0.0f,   cx * sin_cr - cy * cos_cr,
                     0.0f,    0.0f,   1.0f,   0.0f,
                     0.0f,    0.0f,   0.0f,   1.0f
                );

                // Orthographic projection: maps world units to NDC [-1,1] given viewport and zoom
                float z = a_Camera.zoom;
                float W = a_Camera.width  / PIXELS_PER_UNIT;  // viewport width in world units
                float H = a_Camera.height / PIXELS_PER_UNIT;  // viewport height in world units

                Math::Matrix4x4 proj(
                    2.0f * z / W, 0.0f,         0.0f, 0.0f,
                    0.0f,         2.0f * z / H, 0.0f, 0.0f,
                    0.0f,         0.0f,         1.0f, 0.0f,
                    0.0f,         0.0f,         0.0f, 1.0f
                );

                modelMatrix = proj * view * model;
            }

            size_t entityId;
            size_t textureId;
            size_t shaderId;
            Math::Matrix4x4 modelMatrix;
        };
    }
}
