#pragma once
#include <cstdint>
#include "Collider2D.h"

namespace Wheel::Common { class Transform2D; }

namespace Wheel
{
    namespace Collision
    {
        struct RayCastResult
        {
            //nothing hit
            RayCastResult() = default;
            RayCastResult(bool a_IsHit, const Math::Vector2& a_WorldPosition, Common::Transform2D* a_Transform,
                BoxCollider2D* a_BoxCollider = nullptr, CircleCollider2D* a_CircleCollider = nullptr) :
                IsHit(a_IsHit), WorldPosition(a_WorldPosition), Transform(a_Transform), BoxCollider(a_BoxCollider), CircleCollider(a_CircleCollider) {}
            uint32_t EntityID = UINT32_MAX;
            bool IsHit = false;
            //WARNING: Cannot guarantee consistency over multiple frames!
            BoxCollider2D* BoxCollider = nullptr;
            //WARNING: Cannot guarantee consistency over multiple frames!
            CircleCollider2D* CircleCollider = nullptr;
            //WARNING: Cannot guarantee consistency over multiple frames!
            Common::Transform2D* Transform = nullptr;
            Math::Vector2 WorldPosition = Math::Vector2(FLT_MAX, FLT_MAX);
        };

        class RayCast
        {
        public:
            static RayCastResult Raycast(const Math::Vector2& point, const Math::Vector2& direction)
            {
                return RayCastResult();
            }
        };
    }
}
