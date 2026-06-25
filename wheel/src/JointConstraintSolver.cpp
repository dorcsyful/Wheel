#include "../include/systems/helpers/JointConstraintSolver.h"

Wheel::Engine::Physics::JointTempCalculations Wheel::Engine::Physics::JointConstraintSolver::
PrepareJointConstraintSolver(Components::Transform2D& a_Transform1, Components::Transform2D& a_Transform2,
    Components::Rigidbody2D& a_Rigidbody1, Components::Rigidbody2D& a_Rigidbody2, Components::DistanceJoint2D& a_Joint)
{
    JointTempCalculations jtc(a_Transform1, a_Transform2, a_Rigidbody1, a_Rigidbody2, a_Joint);
    return jtc;
}


void Wheel::Engine::Physics::JointConstraintSolver::SolveDistanceJointConstraint(JointTempCalculations& jointCalc,
                                                                                 float deltaTime)
{
    const Math::Vector2& s1 = jointCalc.transform1.GetScale();
    Math::Vector2 local1(jointCalc.joint.localAnchorPoint.x * s1.x, jointCalc.joint.localAnchorPoint.y * s1.y);   // S
    Math::Vector2 anchor1 = jointCalc.transform1.GetPosition() + jointCalc.transform1.GetRotationMatrix() * local1;  // T + R·(S·local)
    Math::Vector2 anchor2 = jointCalc.joint.otherAnchorPoint;
    if (jointCalc.joint.connectedRigidbody != NO_VALUE)
    {
        const Math::Vector2& s2 = jointCalc.transform2.GetScale();
        Math::Vector2 local2(jointCalc.joint.otherAnchorPoint.x * s2.x, jointCalc.joint.otherAnchorPoint.y * s2.y);   // S
        anchor2 = jointCalc.transform2.GetPosition() + jointCalc.transform2.GetRotationMatrix() * local2;  // T + R·(S·local)
    }
    Math::Vector2 delta = anchor2 - anchor1;
    float lengthSquared = delta.LengthSquared();
    if (lengthSquared  >  jointCalc.joint.distance * jointCalc.joint.distance || (jointCalc.joint.maxDistanceOnly && std::abs(lengthSquared) > 0.001))
    {

        Math::Vector2 direction = delta.Normalized();
        float bias = -( 0.01 / deltaTime ) * deltaTime ; // Baumgarte stabilization for now

        float a_cross_direction = (anchor1 - jointCalc.transform1.GetPosition()).Cross(direction);
        if (jointCalc.joint.connectedRigidbody == NO_VALUE)
        {
            Math::Vector2 relativeVelocity = jointCalc.rigidbody1.linearVelocity;
            float constraintMass = jointCalc.rigidbody1.GetInverseMass() + a_cross_direction * a_cross_direction * jointCalc.rigidbody1.GetInverseInertia();
            float impulseMagnitude = -(relativeVelocity + bias).Dot(direction) / constraintMass;
            Math::Vector2 impulse = impulseMagnitude * direction;
            jointCalc.rigidbody1.linearVelocity += impulse * jointCalc.rigidbody1.GetInverseMass();
            jointCalc.rigidbody1.angularVelocity += a_cross_direction * impulseMagnitude * jointCalc.rigidbody1.GetInverseInertia();
        }
        else
        {
            Math::Vector2 relativeVelocity = jointCalc.rigidbody1.linearVelocity - jointCalc.rigidbody2.linearVelocity;
            float b_cross_direction = (anchor2 - jointCalc.transform2.GetPosition()).Cross(direction);
            float constraintMass = jointCalc.rigidbody1.GetInverseMass() + jointCalc.rigidbody2.GetInverseMass() + a_cross_direction * a_cross_direction * jointCalc.rigidbody1.GetInverseInertia()
                    + b_cross_direction * b_cross_direction * jointCalc.rigidbody2.GetInverseInertia();
            float impulseMagnitude = -(relativeVelocity + bias).Dot(direction) / constraintMass;
            Math::Vector2 impulse = impulseMagnitude * direction;
            jointCalc.rigidbody1.linearVelocity += impulse * jointCalc.rigidbody1.GetInverseMass();
            jointCalc.rigidbody1.angularVelocity += a_cross_direction * impulseMagnitude * jointCalc.rigidbody1.GetInverseInertia();
            jointCalc.rigidbody2.linearVelocity -= impulse * jointCalc.rigidbody2.GetInverseMass();
            jointCalc.rigidbody2.angularVelocity -= b_cross_direction * impulseMagnitude * jointCalc.rigidbody2.GetInverseInertia();
        }
    }
}
