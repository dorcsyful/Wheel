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
            E_COLLIDER2_D type = E_COLLIDER2_D::BOX;

            // Field names match the private member names so reflection displays them without a prefix
            REFLECT_BEGIN(BoxCollider2D)
            FIELD(width)
            FIELD(height)
            FIELD(offset)
            REFLECT_END(BoxCollider2D, "BoxCollider2D")

            float GetWidth()  const { return width; }
            float GetHeight() const { return height; }
            const Math::Vector2& GetOffset() const { return offset; }

            void SetWidth(float w) { width = w;    isDirty = true; }
            void SetHeight(float h) { height = h;   isDirty = true; }
            void SetOffset(const Math::Vector2& off) { offset = off; isDirty = true; }

            mutable bool isDirty = true;
            mutable Math::Vector2 cachedVertices[4] {};
            mutable Math::Vector2 cachedNormals[4] {};

        private:
            float width  = 0.0f;
            float height = 0.0f;
            Math::Vector2 offset = Math::Vector2(0.0f, 0.0f);
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