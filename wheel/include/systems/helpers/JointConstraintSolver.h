#pragma once
#include "components/CameraComponent.h"
#include "components/Joint2D.h"
#include "components/Rigidbody2D.h"
#include "components/Transform2D.h"

namespace Wheel
{
    namespace Engine
    {
        namespace Physics
        {
            struct JointTempCalculations
            {
                JointTempCalculations(Components::Transform2D& a_Transform1, Components::Transform2D& a_Transform2,
                    Components::Rigidbody2D& a_Rigidbody1, Components::Rigidbody2D& a_Rigidbody2,
                    Components::DistanceJoint2D& a_Joint)
                    : transform1(a_Transform1), transform2(a_Transform2),
                      rigidbody1(a_Rigidbody1), rigidbody2(a_Rigidbody2),
                      joint(a_Joint) {}
                Components::Transform2D& transform1;
                Components::Transform2D& transform2;
                Components::Rigidbody2D& rigidbody1;
                Components::Rigidbody2D& rigidbody2;
                Components::DistanceJoint2D& joint;
                float a_cross_direction = 0.0f, b_cross_direction = 0.0f;
                Math::Vector2 anchor1, anchor2;
                Math::Vector2 direction;
                float constraintMass = 0.0f;
                float bias = 0.0f;
                bool go = false;
                // Decided once in Prepare: true = solve against both bodies, false =
                // solve body1 against a fixed/static anchor. Avoids recomputing (and
                // having to keep in sync) the branch condition in Solve.
                bool twoBody = false;
                float cachedImpulse = 0.0f;
                uint32_t entity = NO_VALUE;   // joint owner id; key for the warm-start cache
            };

            class JointConstraintSolver
            {
            public:
                static JointTempCalculations PrepareJointConstraintSolver(Components::Transform2D& a_Transform1, Components::Transform2D& a_Transform2,
                    Components::Rigidbody2D& a_Rigidbody1, Components::Rigidbody2D& a_Rigidbody2,
                    Components::DistanceJoint2D& a_Joint, float a_DeltaTime);
                static void SolveDistanceJointConstraint(JointTempCalculations& jointCalc);
                static void WarmStart(JointTempCalculations& jointCalc, float cachedImpulse);
            };
        }
    }
}


