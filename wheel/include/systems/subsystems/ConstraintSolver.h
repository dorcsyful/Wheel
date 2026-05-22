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
            struct TempCalculations
            {
                TempCalculations(Collision::Collision2DManifold& a_Manifold, float a_DeltaTime,
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
            };


            class ConstraintSolver
            {
            public:
                static TempCalculations PrepareConstraintSolver(Collision::Collision2DManifold& a_Manifold,
                    Components::Transform2D& a_ATransform, Components::Transform2D& a_BTransform,
                    Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B,
                    float a_DeltaTime);
                static void SolvePseudoVelocities(TempCalculations& tempCalc);
                static void Solve2ContactConstraint(TempCalculations& tempCalc);
                static void Solve1ContactConstraint(const TempCalculations& tempCalc);
            private:
                static void CalculateImpulses(const Math::Vector2& normal, Math::Matrix2x2 k, float b1, float b2,
                    Math::Vector2& impulse_on_a, Math::Vector2& impulse_on_b,
                    Math::Vector2& lambda);
                static void SolveFrictionConstraint(TempCalculations& a_TempCalc);
            };
        }
    }
}



