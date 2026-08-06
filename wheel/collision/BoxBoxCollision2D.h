#pragma once
#include "Collision2DManifold.h"
#include "Collider2D.h"
#include "common/Transform2D.h"

namespace Wheel
{
    namespace Collision
    {
        class BoxBoxCollision2D
        {
        public:
            static bool CalculateAndUpdateOverlap(const Wheel::Collision::BoxCollider2D& a_Collider1,
                                                  const Wheel::Math::Vector2* vertices1,
                                                  const Wheel::Math::Vector2* vertices2,
                                                  Wheel::Collision::Collision2DManifold manifold, float& minOverlap1,
                                                  int& bestAxis1, int i,
                                                  Wheel::Collision::Collision2DManifold& a_Value);
            static Collision2DManifold BoxBoxCollision(const BoxCollider2D& a_Collider1, const Common::Transform2D& a_Transform1, const BoxCollider2D& a_Collider2, const Common::Transform2D& a_Transform2);
            static const Math::Vector2* GetVertices(const BoxCollider2D& a_Collider, const Common::Transform2D& a_Transform2D);

        private:
            static int GetContactPoints(const BoxCollider2D& a_Collider1, const BoxCollider2D& a_Collider2, int a_Normal1, int a_Normal2, Math::Vector2* a_ContactPoints, uint16_t* a_ContactIds, bool a_Flip);
            static void ClipSegmentToLine(const Math::Vector2& a_P1, const Math::Vector2& a_P2, const Math::Vector2& a_Normal, Math::Vector2 a_Offset, Math::Vector2* a_Contact);
            static void ProjectOntoAxis(const Math::Vector2* a_Vertices, const Math::Vector2& a_Axis, float& a_Min, float& a_Max);
        };
    }
}
