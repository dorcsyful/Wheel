#pragma once
#include "components/Transform2D.h"
#include "helpers/Collision2DManifold.h"


namespace Wheel
{
    namespace Engine
    {
        namespace Collision
        {
            class CircleBoxCollision2D
            {
            public:
                static Collision2DManifold CheckCircleBoxCollision(const Components::Transform2D& a_CircleTransform, const Components::CircleCollider2D& a_CircleCollider,
                    const Components::Transform2D& a_BoxTransform, const Components::BoxCollider2D& a_BoxCollider);
            };
        }
    }
}

