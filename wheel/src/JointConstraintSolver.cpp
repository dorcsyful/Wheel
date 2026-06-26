#include "../include/systems/helpers/JointConstraintSolver.h"

Wheel::Engine::Physics::JointTempCalculations Wheel::Engine::Physics::JointConstraintSolver::
PrepareJointConstraintSolver(Components::Transform2D& a_Transform1, Components::Transform2D& a_Transform2,
    Components::Rigidbody2D& a_Rigidbody1, Components::Rigidbody2D& a_Rigidbody2, Components::DistanceJoint2D& a_Joint, float a_DeltaTime)
{
    float dt = std::max(a_DeltaTime, 1e-6f);

    JointTempCalculations jtc(a_Transform1, a_Transform2, a_Rigidbody1, a_Rigidbody2, a_Joint);

    const Math::Vector2& s1 = jtc.transform1.GetScale();
    Math::Vector2 local1(jtc.joint.localAnchorPoint.x * s1.x, jtc.joint.localAnchorPoint.y * s1.y);   // S
    jtc.anchor1 = jtc.transform1.GetPosition() + jtc.transform1.GetRotationMatrix() * local1;  // T + R·(S·local)
    jtc.anchor2 = jtc.joint.otherAnchorPoint;
    if (jtc.joint.connectedRigidbody != NO_VALUE)
    {
        const Math::Vector2& s2 = jtc.transform2.GetScale();
        Math::Vector2 local2(jtc.joint.otherAnchorPoint.x * s2.x, jtc.joint.otherAnchorPoint.y * s2.y);   // S
        jtc.anchor2 = jtc.transform2.GetPosition() + jtc.transform2.GetRotationMatrix() * local2;  // T + R·(S·local)
    }
    Math::Vector2 delta = (jtc.anchor1 - jtc.anchor2);
    float length = delta.Length();
    if (length  >  jtc.joint.distance || (jtc.joint.maxDistanceOnly && std::abs(length) < 0.001))
    {
        jtc.go = true;
    }
    else
    {
        jtc.go = false;
        return jtc;
    }
    float currentLength = length;
    float C = currentLength - jtc.joint.distance;
    Math::Vector2 direction = (currentLength > 1e-6f)
            ? delta * (1.0f / currentLength)
            : Math::Vector2(1.0f, 0.0f);
    jtc.direction = direction;
    jtc.bias = (0.1 / dt) * C;
    jtc.a_cross_direction = (jtc.anchor1 - jtc.transform1.GetPosition()).Cross(direction);
    jtc.b_cross_direction = 0.0f;
    if (jtc.joint.connectedRigidbody != NO_VALUE && (jtc.rigidbody2.GetType() != Components::Rigidbody2DType::STATIC))
    {
        jtc.twoBody = true;
        jtc.b_cross_direction = (jtc.anchor2 - jtc.transform2.GetPosition()).Cross(direction);
        jtc.constraintMass = jtc.rigidbody1.GetInverseMass() + jtc.rigidbody2.GetInverseMass() + jtc.a_cross_direction * jtc.a_cross_direction * jtc.rigidbody1.GetInverseInertia()
        + jtc.b_cross_direction * jtc.b_cross_direction * jtc.rigidbody2.GetInverseInertia();

    }
    else
    {
        jtc.constraintMass = jtc.rigidbody1.GetInverseMass() + jtc.a_cross_direction * jtc.a_cross_direction * jtc.rigidbody1.GetInverseInertia();

    }
    // No movable mass on the constraint (e.g. a static joint owner) -> dividing by
    // it would produce an infinite impulse. Disable rather than explode.
    if (jtc.constraintMass < 1e-12f)
        jtc.go = false;
    return jtc;
}


void Wheel::Engine::Physics::JointConstraintSolver::SolveDistanceJointConstraint(JointTempCalculations& jointCalc)
{
    // Prepare bails out early (leaving direction/bias/constraintMass unset) when the
    // joint isn't violated, so skip those — solving them reads garbage and NaNs out.
    if (!jointCalc.go)
        return;
    if (!jointCalc.twoBody)
    {
        float relativeVelocity = jointCalc.rigidbody1.linearVelocity.Dot(jointCalc.direction) + jointCalc.a_cross_direction * jointCalc.rigidbody1.angularVelocity;
        float impulseMagnitude = -(relativeVelocity + jointCalc.bias) / jointCalc.constraintMass;
        Math::Vector2 impulse = impulseMagnitude * jointCalc.direction;
        jointCalc.rigidbody1.linearVelocity += impulse * jointCalc.rigidbody1.GetInverseMass();
        jointCalc.rigidbody1.angularVelocity += jointCalc.a_cross_direction * impulseMagnitude * jointCalc.rigidbody1.GetInverseInertia();
    }
    else
    {
        float relativeVelocity = (jointCalc.rigidbody1.linearVelocity - jointCalc.rigidbody2.linearVelocity).Dot(jointCalc.direction) +
            jointCalc.b_cross_direction * jointCalc.rigidbody2.angularVelocity -
                jointCalc.a_cross_direction * jointCalc.rigidbody1.angularVelocity;
        float impulseMagnitude = -(relativeVelocity + jointCalc.bias) / jointCalc.constraintMass;
        Math::Vector2 impulse = impulseMagnitude * jointCalc.direction;
        jointCalc.rigidbody1.linearVelocity += impulse * jointCalc.rigidbody1.GetInverseMass();
        jointCalc.rigidbody1.angularVelocity += jointCalc.a_cross_direction * impulseMagnitude * jointCalc.rigidbody1.GetInverseInertia();
        jointCalc.rigidbody2.linearVelocity -= impulse * jointCalc.rigidbody2.GetInverseMass();
        jointCalc.rigidbody2.angularVelocity -= jointCalc.b_cross_direction * impulseMagnitude * jointCalc.rigidbody2.GetInverseInertia();
    }
}
