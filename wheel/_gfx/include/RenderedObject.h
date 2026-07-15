#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>
#include "../../rendering/CameraComponent.h"
#include "../../rendering/Sprite.h"
#include "common/Transform2D.h"
#include "math/Matrix4x4.h"
#include "math/Vector4.h"
#include "../../rendering/RenderLayers.h"
#include "UniformValue.h"
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

            /**
             * @brief Builds the camera matrix. Called once per frame
             */
            static Math::Matrix4x4 ComputeViewProj(const Common::Transform2D& a_CameraTransform,
                                                   const Rendering::CameraComponent& a_Camera)
            {
                float cos_cr = std::cos(a_CameraTransform.GetRotationInRadians());
                float sin_cr = std::sin(a_CameraTransform.GetRotationInRadians());
                float cx = a_CameraTransform.GetPosition().x;
                float cy = a_CameraTransform.GetPosition().y;

                Math::Matrix4x4 view(
                     cos_cr,  sin_cr, 0.0f, -(cx * cos_cr + cy * sin_cr),
                    -sin_cr,  cos_cr, 0.0f,   cx * sin_cr - cy * cos_cr,
                     0.0f,    0.0f,   1.0f,   0.0f,
                     0.0f,    0.0f,   0.0f,   1.0f
                );

                float z = a_Camera.zoom;
                float H = a_Camera.orthoSize;
                float W = H * (a_Camera.width / a_Camera.height);

                Math::Matrix4x4 proj(
                    2.0f * z / W, 0.0f,         0.0f, 0.0f,
                    0.0f,         2.0f * z / H, 0.0f, 0.0f,
                    0.0f,         0.0f,         1.0f, 0.0f,
                    0.0f,         0.0f,         0.0f, 1.0f
                );

                return proj * view;
            }

            void Add2DComponent(const Rendering::Sprite& a_Render, uint32_t a_EntityId,
                                const Common::Transform2D& a_Transform)
            {
                Build(a_Render, a_EntityId, a_Transform.GetPosition(), a_Transform.GetRotationInRadians(),
                      a_Transform.GetScale());
            }

            // Same as Add2DComponent but takes raw pos/rot/scale so the caller can pass
            // values interpolated between two fixed-timestep physics states.
            void Add2DComponentInterpolated(const Rendering::Sprite& a_Render, uint32_t a_EntityId,
                                            const Math::Vector2& a_Position, float a_Rotation,
                                            const Math::Vector2& a_Scale)
            {
                Build(a_Render, a_EntityId, a_Position, a_Rotation, a_Scale);
            }

            // Builds only the per-object rotation matrix and sets the translation
            void Build(const Rendering::Sprite& a_Render, uint32_t a_EntityId,
                       const Math::Vector2& a_Position, float a_Rotation,
                       const Math::Vector2& a_Scale)
            {
                entityId = a_EntityId;
                color = a_Render.color;

                float cos_r = std::cos(a_Rotation);
                float sin_r = std::sin(a_Rotation);
                float sx = a_Render.width  * a_Scale.x;
                float sy = a_Render.height * a_Scale.y;

                // Math 2x2 stored as a flat float[4] so it's easier to pass to the renderer
                linear[0] =  sx * cos_r;   // col0.x
                linear[1] =  sx * sin_r;   // col0.y
                linear[2] = -sy * sin_r;   // col1.x
                linear[3] =  sy * cos_r;   // col1.y
                translate[0] = a_Position.x;
                translate[1] = a_Position.y;
            }
            void SetRenderLayer(Helpers::RenderLevel a_Level)
            {
                level = a_Level;
            }

            // Per-object uniform override. The renderer uploads these after the
            // engine uniforms, so a feature can drive shader inputs (e.g. an
            // outline's thickness) that vary per entity instead of per material.
            struct UniformOverride
            {
                char name[24] = {};
                MaterialProperty value;
            };
            static constexpr int MAX_UNIFORM_OVERRIDES = 4;

            // Queue a per-object uniform by name
            template<typename T>
            void SetUniform(const char* a_Name, const T& a_Value)
            {
                static_assert(sizeof(T) <= sizeof(MaterialProperty::bytes),
                              "Uniform value is larger than MaterialProperty storage.");
                if (overrideCount >= MAX_UNIFORM_OVERRIDES) return;
                UniformOverride& o = overrides[overrideCount++];
                std::strncpy(o.name, a_Name, sizeof(o.name) - 1);
                o.name[sizeof(o.name) - 1] = '\0';
                o.value.type = UniformTypeOf<T>::value;
                std::memcpy(o.value.bytes, &a_Value, sizeof(T));
            }

            uint32_t entityId  = 0;
            uint32_t textureId = 0;
            uint32_t shaderId  = 0;
            uint32_t materialId = 0;
            Math::Vector4 color = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            float linear[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            float translate[2] = { 0.0f, 0.0f };
            Helpers::RenderLevel level = 0;

            UniformOverride overrides[MAX_UNIFORM_OVERRIDES];
            uint8_t overrideCount = 0;
        };
    }
}