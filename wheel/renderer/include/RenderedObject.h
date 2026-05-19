#pragma once
#include <cmath>
#include <cstdint>
#include "../../wheel/include/core/Globals.h"
#include "components/CameraComponent.h"
#include "components/Render2DComponent.h"
#include "components/Transform2D.h"
#include "math/Matrix4x4.h"
#include "math/Vector4.h"

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
            RenderedObject() = default;
            static Math::Matrix4x4 ComputeViewProj(const Components::Transform2D& a_CameraTransform,
                                                   const Components::CameraComponent& a_Camera)
            {
                constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
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

                float z = a_Camera.zoom;
                float W = a_Camera.width  / PIXELS_PER_UNIT;
                float H = a_Camera.height / PIXELS_PER_UNIT;

                Math::Matrix4x4 proj(
                    2.0f * z / W, 0.0f,         0.0f, 0.0f,
                    0.0f,         2.0f * z / H, 0.0f, 0.0f,
                    0.0f,         0.0f,         1.0f, 0.0f,
                    0.0f,         0.0f,         0.0f, 1.0f
                );

                return proj * view;
            }

            void Add2DComponent(const Components::Render2DComponent& a_Render, uint32_t a_EntityId,
                                const Components::Transform2D& a_Transform)
            {
                Build(a_Render, a_EntityId, a_Transform.GetPosition(), a_Transform.GetRotation(),
                      a_Transform.GetScale());
            }

            // Same as Add2DComponent but takes raw pos/rot/scale so the caller can pass
            // values interpolated between two fixed-timestep physics states.
            void Add2DComponentInterpolated(const Components::Render2DComponent& a_Render, uint32_t a_EntityId,
                                            const Math::Vector2& a_Position, float a_RotationDeg,
                                            const Math::Vector2& a_Scale)
            {
                Build(a_Render, a_EntityId, a_Position, a_RotationDeg, a_Scale);
            }

            // Builds only the compact per-object 2D affine. The view-projection
            // is shared by the whole frame and applied in the vertex shader, so
            // it is no longer folded in here (no per-object 4x4 multiply).
            void Build(const Components::Render2DComponent& a_Render, uint32_t a_EntityId,
                       const Math::Vector2& a_Position, float a_RotationDeg,
                       const Math::Vector2& a_Scale)
            {
                entityId = a_EntityId;
                color    = a_Render.color;

                constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
                float cos_r = std::cos(a_RotationDeg * DEG_TO_RAD);
                float sin_r = std::sin(a_RotationDeg * DEG_TO_RAD);
                float sx = a_Render.width  * a_Scale.x;
                float sy = a_Render.height * a_Scale.y;

                // Math 2x2 is [[sx*cos, -sy*sin], [sx*sin, sy*cos]]. Stored
                // column-major so it feeds straight into glUniformMatrix2fv:
                // {col0.x, col0.y, col1.x, col1.y}.
                linear[0] =  sx * cos_r;   // col0.x
                linear[1] =  sx * sin_r;   // col0.y
                linear[2] = -sy * sin_r;   // col1.x
                linear[3] =  sy * cos_r;   // col1.y
                translate[0] = a_Position.x;
                translate[1] = a_Position.y;
            }

            uint32_t entityId  = 0;
            uint32_t textureId = 0;
            uint32_t shaderId  = 0;
            Math::Vector4 color = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            float linear[4]    = { 1.0f, 0.0f, 0.0f, 1.0f };
            float translate[2] = { 0.0f, 0.0f };
        };
    }
}