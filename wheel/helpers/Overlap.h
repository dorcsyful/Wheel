#pragma once
#include "collision/Collider2D.h"
#include "rendering/Sprite.h"
#include "../common/Transform2D.h"
#include "math/Vector2.h"

namespace Wheel
{
    namespace Helpers
    {
        inline bool PointInSprite(const Math::Vector2& a_WorldPoint, const Common::Transform2D& a_Transform,
                                  const Rendering::Sprite& a_Sprite)
        {
            Math::Matrix2x2 rotation = a_Transform.GetRotationMatrix();
            Math::Vector2 localized = a_WorldPoint - a_Transform.GetPosition();
            localized = rotation.Inverse() * localized;
            Math::Vector2 scaledSize = Math::Vector2(a_Sprite.width * a_Transform.GetScale().x, a_Sprite.height * a_Transform.GetScale().y);
            if (localized.x > -scaledSize.x / 2.f && localized.x < scaledSize.x / 2.f)
            {
                if (localized.y > -scaledSize.y / 2.f && localized.y < scaledSize.y / 2.f)
                {
                    return true;
                }
            }
            return false;
        }
        bool PointInCollider(const Math::Vector2& point, const Common::Transform2D& transform,
            const Collision::BoxCollider2D& collider);
        bool PointInCollider(const Math::Vector2& point, const Common::Transform2D& transform,
            const Collision::CircleCollider2D& collider);
    }
}
