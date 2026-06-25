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
            };

            class JointConstraintSolver
            {
            public:
                static JointTempCalculations PrepareJointConstraintSolver(Components::Transform2D& a_Transform1, Components::Transform2D& a_Transform2,
                    Components::Rigidbody2D& a_Rigidbody1, Components::Rigidbody2D& a_Rigidbody2,
                    Components::DistanceJoint2D& a_Joint);
                static void SolveDistanceJointConstraint(JointTempCalculations& jointCalc, float deltaTime);
            };
        }
    }
}


