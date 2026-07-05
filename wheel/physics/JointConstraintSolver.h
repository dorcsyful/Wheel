#pragma once
#include "../rendering/CameraComponent.h"
#include "Joint2D.h"
#include "Rigidbody2D.h"
#include "common/Transform2D.h"

namespace Wheel
{
    namespace Physics
    {
        struct JointTempCalculations
        {
            JointTempCalculations(Common::Transform2D& a_Transform1, Common::Transform2D& a_Transform2,
                Rigidbody2D& a_Rigidbody1, Rigidbody2D& a_Rigidbody2, DistanceJoint2D& a_Joint)
                : transform1(a_Transform1), transform2(a_Transform2),
                  rigidbody1(a_Rigidbody1), rigidbody2(a_Rigidbody2),
                  joint(a_Joint) {}
            Common::Transform2D& transform1;
            Common::Transform2D& transform2;
            Rigidbody2D& rigidbody1;
            Rigidbody2D& rigidbody2;
            DistanceJoint2D& joint;
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
            static JointTempCalculations PrepareJointConstraintSolver(Common::Transform2D& a_Transform1, Common::Transform2D& a_Transform2,
                Rigidbody2D& a_Rigidbody1, Rigidbody2D& a_Rigidbody2,
                DistanceJoint2D& a_Joint, float a_DeltaTime);
            static void SolveDistanceJointConstraint(JointTempCalculations& jointCalc);
            static void WarmStart(JointTempCalculations& jointCalc, float cachedImpulse);
        };
    }
}


