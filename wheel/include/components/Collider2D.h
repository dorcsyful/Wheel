#pragma once
#include "math/Vector2.h"

namespace Wheel
{
    namespace Components
    {
        enum class E_COLLIDER2_D
        {
            NONE,
            BOX,
            CIRCLE
        };

        struct BoxCollider2D
        {
            float width = 0.0f;
            float height = 0.0f;
            Math::Vector2 offset = { 0.0f, 0.0f };
            const E_COLLIDER2_D type = E_COLLIDER2_D::BOX;
        };

        struct CircleCollider2D
        {
            float radius = 0.0f;
            Math::Vector2 offset = { 0.0f, 0.0f };
            const E_COLLIDER2_D type = E_COLLIDER2_D::CIRCLE;

        };
    }
}
