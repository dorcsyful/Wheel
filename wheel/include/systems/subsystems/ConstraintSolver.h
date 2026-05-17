#pragma once
#include <vector>

#include "components/CameraComponent.h"
#include "components/Transform2D.h"
#include "math/Matrix2x2.h"
#include "math/Vector2.h"

namespace Wheel::Components
{
    struct Rigidbody2D;
}

namespace Wheel::Engine::Collision
{
    struct Collision2DManifold;
}

namespace Wheel
{
    namespace Engine
    {
        namespace Physics
        {

            class ConstraintSolver
            {
            public:
                static void Solve2ContactConstraint(const Collision::Collision2DManifold& a_Manifold, float a_DeltaTime,
                    const Components::Transform2D& a_ATransform, const Components::Transform2D& a_BTransform,
                    Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B);
                static void Solve1ContactConstraint(const Math::Vector2& a_ContactPoint, const Math::Vector2& a_CollisionNormal,
                    float a_PenetrationDepth, float a_DeltaTime,
                    const Components::Transform2D& a_ATransform, const Components::Transform2D& a_BTransform,
                    Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B);
            private:
            };
        }
    }
}



