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
                static Collision2DManifold BoxBoxCollision(Components::BoxCollider2D& a_Collider1, const Components::Transform2D& a_Transform1, Components::BoxCollider2D& a_Collider2, const Components::Transform2D& a_Transform2);

            private:
                static void GetVertices(const Components::BoxCollider2D& a_Collider, const Components::Transform2D& a_Transform2D, Math::Vector2* a_Vertices);
                static void ProjectOntoAxis(const Math::Vector2* a_Vertices, const Math::Vector2& a_Axis, float& a_Min, float& a_Max);
            };
        }
    }
}
