#pragma once
#include <stdint.h>
#include "math/Vector2.h"
#include <cfloat>
namespace Wheel
{
    namespace Collision
    {
        struct Collision2DManifold
        {
        public:
            Collision2DManifold() = default;
            Collision2DManifold(uint32_t a_Entity1, uint32_t a_Entity2) : collider1(a_Entity1), collider2(a_Entity2) {}
            bool isColliding = false;
            float penetrationDepth[2] = { FLT_MAX, FLT_MAX };
            float restitutionBias[2] = { FLT_MAX, FLT_MAX };
            Math::Vector2 collisionNormal = Math::Vector2(0.0f, 0.0f);
            Math::Vector2 tangentNormal = Math::Vector2(0.0f, 0.0f);
            Math::Vector2 contactPoint[2] = { Math::Vector2(FLT_MAX, FLT_MAX), Math::Vector2(FLT_MAX, FLT_MAX) };
            // Stable per-contact feature ID (packed reference/incident edges + endpoint).
            uint16_t contactId[2] = { 0xFFFF, 0xFFFF };
            int contactCount = 0;
            uint32_t collider1;
            uint32_t collider2;
        };
    }
}
