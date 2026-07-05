#pragma once
#include "math/Vector2.h"
namespace Wheel
{
    namespace Collision
    {
        struct AABB
        {
        public:
            AABB() = default;
            AABB(const Math::Vector2& a_Min, const Math::Vector2& a_Max) : min(a_Min), max(a_Max) {}

            Math::Vector2 min;
            Math::Vector2 max;

        };
    }
}