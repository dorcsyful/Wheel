#pragma once
#include "helpers/Collision2DManifold.h"
#include "components/Collider2D.h"
#include "components/Transform2D.h"

namespace Wheel
{
    namespace Engine
    {
        namespace Collision
        {
            class BoxBoxCollision2D
            {
            public:
                static Collision2DManifold BoxBoxCollision(const Components::BoxCollider2D& a_Collider1, const Components::Transform2D& a_Transform1, const Components::BoxCollider2D& a_Collider2, const Components::Transform2D& a_Transform2);
                static const Math::Vector2* GetVertices(const Components::BoxCollider2D& a_Collider, const Components::Transform2D& a_Transform2D);

            private:
                static int GetContactPoints(const Components::BoxCollider2D& a_Collider1, const Components::BoxCollider2D& a_Collider2, int a_Normal1, int a_Normal2, Math::Vector2* a_ContactPoints, uint16_t* a_ContactIds, bool a_Flip);
                static void ClipSegmentToLine(const Math::Vector2& a_P1, const Math::Vector2& a_P2, const Math::Vector2& a_Normal, Math::Vector2 a_Offset, Math::Vector2* a_Contact);
                static void ProjectOntoAxis(const Math::Vector2* a_Vertices, const Math::Vector2& a_Axis, float& a_Min, float& a_Max);
            };
        }
    }
}
