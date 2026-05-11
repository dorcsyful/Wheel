#pragma once
#include "components/Collider2D.h"
#include "components/TextureComponent.h"
#include "math/Vector2.h"

namespace Wheel
{
    namespace Engine
    {
        namespace Collision
        {
            struct Collision2DManifold
            {
            public:
                Collision2DManifold(Components::BoxCollider2D& a_Collider1, Components::BoxCollider2D& a_Collider2) : collider1(a_Collider1), collider2(a_Collider2) {}
                bool isColliding = false;
                float penetrationDepth = FLT_MAX;
                Math::Vector2 collisionNormal = Math::Vector2(0.0f, 0.0f);
                Math::Vector2 contactPoint[2] = { Math::Vector2(FLT_MAX, FLT_MAX), Math::Vector2(FLT_MAX, FLT_MAX) };
                Components::BoxCollider2D& collider1;
                Components::BoxCollider2D& collider2;
            };
        }
    }
}
