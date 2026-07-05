#pragma once
#include "common/Transform2D.h"
#include "Collision2DManifold.h"

namespace Wheel
{
    namespace Collision
    {
        class CircleCircleCollision2D
        {
        public:
            static Collision2DManifold CheckCircleCircleCollision(
                const Common::Transform2D& a_ATransform, const Common::Transform2D& a_BTransform,
                const CircleCollider2D& a_ACollider, const CircleCollider2D& a_BCollider);
        };

    }
}
