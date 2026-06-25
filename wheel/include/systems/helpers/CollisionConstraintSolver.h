#pragma once
#include <vector>

#include "components/CameraComponent.h"
#include "components/Transform2D.h"
#include "math/Matrix2x2.h"
#include "math/Vector2.h"
#include "components/Rigidbody2D.h"

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
            struct CollisionTempCalculations
            {
                CollisionTempCalculations(Collision::Collision2DManifold& a_Manifold, float a_DeltaTime,
                    Components::Transform2D& a_ATransform, Components::Transform2D& a_BTransform,
                    Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B)
                        :                     Manifold(a_Manifold), a_Transform(a_ATransform), b_Transform(a_BTransform),
                                              a_Rigidbody(a_A), b_Rigidbody(a_B), DeltaTime(a_DeltaTime) {}
                Collision::Collision2DManifold& Manifold;
                Components::Transform2D& a_Transform;
                Components::Transform2D& b_Transform;
                Components::Rigidbody2D& a_Rigidbody;
                Components::Rigidbody2D& b_Rigidbody;
                float DeltaTime;
                Math::Matrix2x2 k;
                Math::Vector2 b_distance_from_com[2];
                Math::Vector2 a_distance_from_com[2];
                float a_cross_n[2] = { 0.0f, 0.0f };
                float b_cross_n[2] = { 0.0f, 0.0f };
                // Accumulated normal and friction impulses, kept as running totals so
                // each solver iteration clamps against them and they can be cached for
                // next-frame warm starting.
                float impulses[2]         = { 0.0f, 0.0f };
                float frictionImpulses[2] = { 0.0f, 0.0f };
            };


            class CollisionConstraintSolver
            {
            public:
                static CollisionTempCalculations PrepareConstraintSolver(Collision::Collision2DManifold& a_Manifold,
                    Components::Transform2D& a_ATransform, Components::Transform2D& a_BTransform,
                    Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B,
                    float a_DeltaTime);
                // Applies last frame's accumulated impulses to the bodies and seeds the
                // accumulators, so the iterative solve starts near its converged state.
                static void WarmStart(CollisionTempCalculations& tempCalc, const float* a_NormalImpulse, const float* a_FrictionImpulse);
                static void SolvePseudoVelocities(CollisionTempCalculations& tempCalc);
                static void Solve2ContactConstraint(CollisionTempCalculations& tempCalc);
                static void Solve1ContactConstraint(CollisionTempCalculations& tempCalc);
                static void SolveFrictionConstraint(CollisionTempCalculations& a_TempCalc, int i);
            };
        }
    }
}



