#pragma once
#include "rendering/CameraComponent.h"
#include "common/Transform2D.h"
#include "math/Matrix4x4.h"
#include "math/Vector2.h"

namespace Wheel
{
    namespace Helpers
    {
        class Coordinates
        {
            public:
            static Math::Vector2 WorldToScreenCoordinates(const Math::Vector2& worldCoordinates, const Rendering::CameraComponent& a_CameraComponent, const Common::Transform2D& a_CameraTransform)
            {
                float cos_r = std::cos(a_CameraTransform.GetRotationInRadians());
                float sin_r = std::sin(a_CameraTransform.GetRotationInRadians());
                float H = a_CameraComponent.orthoSize;
                float W = H * (a_CameraComponent.width / a_CameraComponent.height);

                float dx = worldCoordinates.x - a_CameraTransform.GetPosition().x;
                float dy = worldCoordinates.y - a_CameraTransform.GetPosition().y;
                float vx =  cos_r * dx + sin_r * dy;
                float vy = -sin_r * dx + cos_r * dy;
                float ndcX = vx * 2.0f * a_CameraComponent.zoom / W;
                float ndcY = vy * 2.0f * a_CameraComponent.zoom / H;
                return { (ndcX + 1.0f) * a_CameraComponent.width * 0.5f, (1.0f - ndcY) * a_CameraComponent.height * 0.5f };

            }
            static Math::Vector2 ScreenToWorldCoordinates(const Math::Vector2& screenCoordinates, const Rendering::CameraComponent& a_CameraComponent, const Common::Transform2D& a_CameraTransform)
            {
                float cos_r = std::cos(a_CameraTransform.GetRotationInRadians());
                float sin_r = std::sin(a_CameraTransform.GetRotationInRadians());
                float H = a_CameraComponent.orthoSize;
                float W = H * (a_CameraComponent.width / a_CameraComponent.height);

                float ndcX = (screenCoordinates.x / (a_CameraComponent.width * 0.5f)) - 1.0f;
                float ndcY = 1.0f - (screenCoordinates.y / (a_CameraComponent.height * 0.5f));

                float vx = ndcX * W / (2.0f * a_CameraComponent.zoom);
                float vy = ndcY * H / (2.0f * a_CameraComponent.zoom);

                float dx = cos_r * vx - sin_r * vy;
                float dy = sin_r * vx + cos_r * vy;

                return { dx + a_CameraTransform.GetPosition().x, dy + a_CameraTransform.GetPosition().y };
            }
        };
    }
}
