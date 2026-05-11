#pragma once
#include "DebugDescriptor.h"
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
            Math::Vector2 offset = Math::Vector2(0.0f, 0.0f);
            E_COLLIDER2_D type = E_COLLIDER2_D::BOX;

            REFLECT_BEGIN(BoxCollider2D)
            FIELD(width)
            FIELD(height)
            FIELD(offset)
            REFLECT_END(BoxCollider2D, "BoxCollider2D")
        };

        struct CircleCollider2D
        {
            float radius = 0.0f;
            Math::Vector2 offset = { 0.0f, 0.0f };
            E_COLLIDER2_D type = E_COLLIDER2_D::CIRCLE;

            REFLECT_BEGIN(CircleCollider2D)
            FIELD(radius)
            FIELD(offset)
            REFLECT_END(CircleCollider2D, "CircleCollider2D")

        };
    }
}
