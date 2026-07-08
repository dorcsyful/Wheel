#pragma once
#include <cfloat>
#include "math/Vector2.h"

#include "debugger/DebugDescriptor.h"
namespace Wheel
{
    namespace Collision
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


            float GetWidth()  const { return width; }
            float GetHeight() const { return height; }
            const Math::Vector2& GetOffset() const { return offset; }

            void SetWidth(float w) { width = w;    isDirty = true; }
            void SetHeight(float h) { height = h;   isDirty = true; }
            void SetOffset(const Math::Vector2& off) { offset = off; isDirty = true; }

            void GetAABB(Math::Vector2& a_Out_Min, Math::Vector2& a_Out_Max) const
            {
                a_Out_Min = cachedVertices[0];
                a_Out_Max = cachedVertices[0];
                for (int i = 0; i <4;i++)
                {
                    if (cachedVertices[i].x < a_Out_Min.x) a_Out_Min.x = cachedVertices[i].x;
                    if (cachedVertices[i].x > a_Out_Max.x) a_Out_Max.x = cachedVertices[i].x;
                    if (cachedVertices[i].y < a_Out_Min.y) a_Out_Min.y = cachedVertices[i].y;
                    if (cachedVertices[i].y > a_Out_Max.y) a_Out_Max.y = cachedVertices[i].y;
                }
            }

            mutable bool isDirty = true;
            //World position, updated when the entity is transformed in any way
            mutable Math::Vector2 cachedVertices[4] {};
            mutable Math::Vector2 cachedNormals[4] {};
            REFLECT_BEGIN(BoxCollider2D)
            FIELD(width)
            FIELD(height)
            FIELD(offset)
            REFLECT_END(BoxCollider2D, "BoxCollider2D")
        private:
            float width  = 0.0f;
            float height = 0.0f;
            Math::Vector2 offset = Math::Vector2(0.0f, 0.0f);


        };

        struct CircleCollider2D
        {
            //Note: when the transform is non-uniform the smallest of value will be multiplied by the radius
            float radius = 0.0f;
            Math::Vector2 offset = { 0.0f, 0.0f };
            E_COLLIDER2_D type = E_COLLIDER2_D::CIRCLE;
            mutable bool isDirty = true;

            void GetAABB(Math::Vector2& a_Out_Min, Math::Vector2& a_Out_Max, const Math::Vector2& a_Position) const
            {
                a_Out_Min.x =a_Position.x + offset.x - radius;
                a_Out_Min.y =a_Position.y + offset.y - radius;
                a_Out_Max.x =a_Position.x + offset.x + radius;
                a_Out_Max.y =a_Position.y + offset.y + radius;
            }

            REFLECT_BEGIN(CircleCollider2D)
            FIELD(radius)
            FIELD(offset)
            REFLECT_END(CircleCollider2D, "CircleCollider2D")
        };
    }
}