#include "../include/systems/subsystems/ConstraintSolver.h"

#include "components/Rigidbody2D.h"
#include "helpers/Collision2DManifold.h"

void Wheel::Engine::Physics::ConstraintSolver::CalculateImpulses(Wheel::Math::Vector2 normal, Wheel::Math::Matrix2x2 k, float b1, float b2, Wheel::Math::Vector2& impulse_on_a, Wheel::Math::Vector2& impulse_on_b, Wheel::Math::Vector2& lambda)
{
    if (lambda[0] > 0 && lambda[1] > 0)
    {
        impulse_on_a = -normal * (lambda[0] + lambda[1]);
        impulse_on_b = normal * (lambda[0] + lambda[1]);
    }
    else if (lambda[0] > 0 && lambda[1] <= 0)
    {
        lambda[0] = std::max(b1 / k[0][0], 0.0f); // No impulse at contact point 2
        lambda[1] = 0.0f;
        impulse_on_a = -normal * (lambda[0] + lambda[1]);
        impulse_on_b = normal * (lambda[0] + lambda[1]);

    }
    else if (lambda[0] <= 0 && lambda[1] > 0)
    {
        lambda[1] = std::max(b2 / k[1][1], 0.0f); // No impulse at contact point 1
        lambda[0] = 0.0f;
        impulse_on_a = -normal * (lambda[0] + lambda[1]);
        impulse_on_b = normal * (lambda[0] + lambda[1]);
    }
    else
    {
        lambda[0] = 0.0f;
        lambda[1] = 0.0f;
        impulse_on_a = -normal * (lambda[0] + lambda[1]);
        impulse_on_b = normal * (lambda[0] + lambda[1]);
    }
}

Wheel::Engine::Physics::TempCalculations Wheel::Engine::Physics::ConstraintSolver::PrepareConstraintSolver(Collision::Collision2DManifold& a_Manifold,
    Components::Transform2D& a_ATransform, Components::Transform2D& a_BTransform,Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B,
    float a_DeltaTime)
{
    TempCalculations tempCalc(a_Manifold, a_DeltaTime, a_ATransform, a_BTransform, a_A, a_B);
    Math::Matrix2x2 R_a = a_ATransform.GetRotationMatrix();
    tempCalc.a_distance_from_com[0] = (a_Manifold.contactPoint[0] - a_ATransform.GetPosition()) - R_a * a_A.centerOfMass;
    if (a_Manifold.contactCount > 1) tempCalc.a_distance_from_com[1] = (a_Manifold.contactPoint[1] - a_ATransform.GetPosition()) - R_a * a_A.centerOfMass;

    Math::Matrix2x2 R_b = a_BTransform.GetRotationMatrix();
    tempCalc.b_distance_from_com[0] = (a_Manifold.contactPoint[0] - a_BTransform.GetPosition()) - R_b * a_B.centerOfMass;
    if (a_Manifold.contactCount > 1) tempCalc.b_distance_from_com[1] =(a_Manifold.contactPoint[1] - a_BTransform.GetPosition()) - R_b * a_B.centerOfMass;

    float invMassA = a_A.GetInverseMass();
    float invMassB = a_B.GetInverseMass();
    Math::Vector2 normal = a_Manifold.collisionNormal;
    for (int i = 0; i < a_Manifold.contactCount; i++)
    {
        for (int j = 0; j < a_Manifold.contactCount; j++)
        {
            //How much a unit impulse at contact point j affects the relative velocity at contact point i
            float aCrossN =(tempCalc.a_distance_from_com[i].Cross(normal) * (tempCalc.a_distance_from_com[j].Cross(normal)));
            float bCrossN =(tempCalc.b_distance_from_com[i].Cross(normal) * (tempCalc.b_distance_from_com[j].Cross(normal)));
            tempCalc.k[i][j] = invMassA + invMassB + aCrossN * a_A.GetInverseInertia() + bCrossN * a_B.GetInverseInertia();
        }
    }
    return tempCalc;
}

void Wheel::Engine::Physics::ConstraintSolver::SolvePseudoVelocities(TempCalculations& tempCalc)
{
    //(vA + ωA × rA - vB - ωB × rB)
    // Position-only solve: the pseudo correction depends solely on the
    // geometric overlap, never on real velocity (the velocity solver owns
    // that). SLOP leaves a small allowed penetration so resting bodies
    // don't fight sub-millimetre jitter every frame.
    const float SLOP = 0.01f;
    for (int i = 0; i < tempCalc.Manifold.contactCount; i++)
    {
        const float MAX_LINEAR_CORRECTION = 0.2f;
        float corr = std::min(std::max(tempCalc.Manifold.penetrationDepth[i] - SLOP, 0.0f),
                              MAX_LINEAR_CORRECTION);
        float biasVel = corr / tempCalc.DeltaTime;
        float lambda  = biasVel / tempCalc.k[i][i];   // biasVel >= 0, so lambda >= 0
        float invMassA = tempCalc.a_Rigidbody.GetInverseMass();
        float invMassB = tempCalc.b_Rigidbody.GetInverseMass();
        Math::Vector2 normal = tempCalc.Manifold.collisionNormal;

        // Same structure (and signs) as Solve1ContactConstraint, written
        // into the throwaway pseudo channel instead of real velocity.
        if (invMassA > 0.0f)
        {
            tempCalc.a_Rigidbody.pseudoLinearVelocity  -= normal * (lambda * invMassA);
            tempCalc.a_Rigidbody.pseudoAngularVelocity -= tempCalc.a_distance_from_com[i].Cross(normal) * lambda * tempCalc.a_Rigidbody.GetInverseInertia();
        }
        if (invMassB > 0.0f)
        {
            tempCalc.b_Rigidbody.pseudoLinearVelocity  += normal * (lambda * invMassB);
            tempCalc.b_Rigidbody.pseudoAngularVelocity += tempCalc.b_distance_from_com[i].Cross(normal) * lambda * tempCalc.b_Rigidbody.GetInverseInertia();
        }
    }
}

void Wheel::Engine::Physics::ConstraintSolver::Solve2ContactConstraint(TempCalculations& tempCalc)
{
    Math::Vector2 normal = tempCalc.Manifold.collisionNormal;

    Math::Vector2 a_velocity = tempCalc.a_Rigidbody.linearVelocity;
    float a_angular_velocity = tempCalc.a_Rigidbody.angularVelocity;

    Math::Vector2 b_velocity = tempCalc.b_Rigidbody.linearVelocity;
    float b_angular_velocity = tempCalc.b_Rigidbody.angularVelocity;

    float invMassA = tempCalc.a_Rigidbody.GetInverseMass();
    float invMassB = tempCalc.b_Rigidbody.GetInverseMass();

    //Velocities at the contact points
    float jq1 = normal.x * b_velocity.x + normal.y * b_velocity.y +
        (tempCalc.b_distance_from_com[0].Cross(normal)) * b_angular_velocity - normal.x * a_velocity.x - normal.y * a_velocity.y -
        (tempCalc.a_distance_from_com[0].Cross(normal)) * a_angular_velocity;
    float jq2 = normal.x * b_velocity.x + normal.y * b_velocity.y +
        (tempCalc.b_distance_from_com[1].Cross(normal)) * b_angular_velocity - normal.x * a_velocity.x - normal.y * a_velocity.y -
        (tempCalc.a_distance_from_com[1].Cross(normal)) * a_angular_velocity;
    float det = tempCalc.k[0][0] * tempCalc.k[1][1] - tempCalc.k[0][1] * tempCalc.k[1][0];
    if (std::abs(det) < 1e-8f)
    {
        // Contact points coincide -> K is singular and Inverse() would return
        // identity, producing a garbage impulse. Solve as a single contact.
        Solve1ContactConstraint(tempCalc);
        return;
    }

    Math::Matrix2x2 k_inverse = tempCalc.k.Inverse();
    float b1 = (-jq1);
    float b2 = (-jq2);
    Math::Vector2 impulse_on_a;
    Math::Vector2 impulse_on_b;
    Math::Vector2 lambda = k_inverse * Math::Vector2(b1, b2);

    CalculateImpulses(normal, tempCalc.k, b1, b2, impulse_on_a, impulse_on_b, lambda);

    if (invMassA > 0.0f)
    {
        tempCalc.a_Rigidbody.linearVelocity += impulse_on_a * invMassA;
        tempCalc.a_Rigidbody.angularVelocity -= tempCalc.a_distance_from_com[0].Cross(normal) * lambda.x * tempCalc.a_Rigidbody.GetInverseInertia();
        tempCalc.a_Rigidbody.angularVelocity -= tempCalc.a_distance_from_com[1].Cross(normal) * lambda.y * tempCalc.a_Rigidbody.GetInverseInertia();
    }
    if (invMassB > 0.0f)
    {
        tempCalc.b_Rigidbody.linearVelocity += impulse_on_b * invMassB;
        tempCalc.b_Rigidbody.angularVelocity += tempCalc.b_distance_from_com[0].Cross(normal) * lambda.x * tempCalc.b_Rigidbody.GetInverseInertia();
        tempCalc.b_Rigidbody.angularVelocity += tempCalc.b_distance_from_com[1].Cross(normal) * lambda.y * tempCalc.b_Rigidbody.GetInverseInertia();
    }
}

void Wheel::Engine::Physics::ConstraintSolver::Solve1ContactConstraint(TempCalculations& tempCalc)
{
        Math::Vector2 normal = tempCalc.Manifold.collisionNormal;

        Math::Vector2 a_velocity = tempCalc.a_Rigidbody.linearVelocity;
        float a_angular_velocity = tempCalc.a_Rigidbody.angularVelocity;

        Math::Vector2 b_velocity = tempCalc.b_Rigidbody.linearVelocity;
        float b_angular_velocity = tempCalc.b_Rigidbody.angularVelocity;

        float invMassA = tempCalc.a_Rigidbody.GetInverseMass();
        float invMassB = tempCalc.b_Rigidbody.GetInverseMass();

        float denominator = invMassA + invMassB +
            std::pow(tempCalc.a_distance_from_com[0].Cross(normal),2) * tempCalc.a_Rigidbody.GetInverseInertia() +
                std::pow(tempCalc.b_distance_from_com[0].Cross(normal),2) * tempCalc.b_Rigidbody.GetInverseInertia();
        float jq = normal.x * b_velocity.x + normal.y * b_velocity.y + (tempCalc.b_distance_from_com[0].Cross(normal)) * b_angular_velocity -
            normal.x * a_velocity.x - normal.y * a_velocity.y - (tempCalc.a_distance_from_com[0].Cross(normal)) * a_angular_velocity;
        float lambda = std::max((-jq) / denominator,0.f);

        Math::Vector2 impulse_on_a = -normal * lambda;
        Math::Vector2 impulse_on_b = normal * lambda;

        if (invMassA > 0.0f)
        {
            tempCalc.a_Rigidbody.linearVelocity += impulse_on_a * invMassA;
            tempCalc.a_Rigidbody.angularVelocity -= tempCalc.a_distance_from_com[0].Cross(normal) * lambda * tempCalc.a_Rigidbody.GetInverseInertia();
        }
        if (invMassB > 0.0f)
        {
            tempCalc.b_Rigidbody.linearVelocity += impulse_on_b * invMassB;
            tempCalc.b_Rigidbody.angularVelocity += tempCalc.b_distance_from_com[0].Cross(normal) * lambda * tempCalc.b_Rigidbody.GetInverseInertia();
        }

}
