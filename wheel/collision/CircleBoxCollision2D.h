#pragma once
#include "common/Transform2D.h"
#include "Collision2DManifold.h"


namespace Wheel
{
    namespace Collision
    {
        struct CircleCollider2D;
        struct BoxCollider2D;

        class CircleBoxCollision2D
        {
        public:
            static Collision2DManifold CheckCircleBoxCollision(const Common::Transform2D& a_CircleTransform, const CircleCollider2D& a_CircleCollider,
                    const Common::Transform2D& a_BoxTransform, const BoxCollider2D& a_BoxCollider);
        };
    }
}

