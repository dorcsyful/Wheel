#include "../include/systems/subsystems/ConstraintSolver.h"

#include <algorithm>

#include "components/Rigidbody2D.h"
#include "helpers/Collision2DManifold.h"

void Wheel::Engine::Physics::ConstraintSolver::CalculateImpulses(const Wheel::Math::Vector2& normal, Wheel::Math::Matrix2x2 k, float b1, float b2, Wheel::Math::Vector2& impulse_on_a, Wheel::Math::Vector2& impulse_on_b, Wheel::Math::Vector2& lambda)
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
        impulse_on_a = Math::Vector2(0.0f, 0.0f);
        impulse_on_b = Math::Vector2(0.0f, 0.0f);
    }
}

void Wheel::Engine::Physics::ConstraintSolver::SolveFrictionConstraint(TempCalculations& a_TempCalc, int i)
{
    float a_cross_tangent = a_TempCalc.a_distance_from_com[i].Cross(a_TempCalc.Manifold.tangentNormal);
    float b_cross_tangent = a_TempCalc.b_distance_from_com[i].Cross(a_TempCalc.Manifold.tangentNormal);
    float k = a_TempCalc.a_Rigidbody.GetInverseMass() + a_TempCalc.b_Rigidbody.GetInverseMass() +
        a_cross_tangent * a_cross_tangent * a_TempCalc.a_Rigidbody.GetInverseInertia() +
        b_cross_tangent * b_cross_tangent * a_TempCalc.b_Rigidbody.GetInverseInertia();
    float dvn = (a_TempCalc.b_Rigidbody.linearVelocity - a_TempCalc.a_Rigidbody.linearVelocity).Dot(a_TempCalc.Manifold.tangentNormal);
    float jq1 = dvn + b_cross_tangent * a_TempCalc.b_Rigidbody.angularVelocity - a_cross_tangent * a_TempCalc.a_Rigidbody.angularVelocity;
    float lambda = -jq1 /k;

    float max_force = std::sqrt(a_TempCalc.a_Rigidbody.friction * a_TempCalc.b_Rigidbody.friction) * a_TempCalc.impulses[i];
    lambda = std::clamp(lambda, -max_force, max_force);
    a_TempCalc.a_Rigidbody.linearVelocity += a_TempCalc.Manifold.tangentNormal * -lambda * a_TempCalc.a_Rigidbody.GetInverseMass();
    a_TempCalc.b_Rigidbody.linearVelocity += a_TempCalc.Manifold.tangentNormal * lambda * a_TempCalc.b_Rigidbody.GetInverseMass();
    a_TempCalc.a_Rigidbody.angularVelocity -= a_cross_tangent * lambda * a_TempCalc.a_Rigidbody.GetInverseInertia();
    a_TempCalc.b_Rigidbody.angularVelocity += b_cross_tangent * lambda * a_TempCalc.b_Rigidbody.GetInverseInertia();

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
    float invInertiaA = a_A.GetInverseInertia();
    float invInertiaB = a_B.GetInverseInertia();
    Math::Vector2 normal = a_Manifold.collisionNormal;
    for (int i = 0; i < a_Manifold.contactCount; i++)
    {
        tempCalc.a_cross_n[i] = tempCalc.a_distance_from_com[i].Cross(normal);
        tempCalc.b_cross_n[i] = tempCalc.b_distance_from_com[i].Cross(normal);
    }
    for (int i = 0; i < a_Manifold.contactCount; i++)
    {
        for (int j = 0; j < a_Manifold.contactCount; j++)
        {
            //How much a unit impulse at contact point j affects the relative velocity at contact point i
            tempCalc.k[i][j] = invMassA + invMassB
                + tempCalc.a_cross_n[i] * tempCalc.a_cross_n[j] * invInertiaA
                + tempCalc.b_cross_n[i] * tempCalc.b_cross_n[j] * invInertiaB;
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
    for (int i = 0; i < tempCalc.Manifold.contactCount; i++)
    {
        const float SLOP = 0.01f;
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
            tempCalc.a_Rigidbody.pseudoAngularVelocity -= tempCalc.a_cross_n[i] * lambda * tempCalc.a_Rigidbody.GetInverseInertia();
        }
        if (invMassB > 0.0f)
        {
            tempCalc.b_Rigidbody.pseudoLinearVelocity  += normal * (lambda * invMassB);
            tempCalc.b_Rigidbody.pseudoAngularVelocity += tempCalc.b_cross_n[i] * lambda * tempCalc.b_Rigidbody.GetInverseInertia();
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
    float dvn = (b_velocity - a_velocity).Dot(normal);
    float jq1 = dvn + tempCalc.b_cross_n[0] * b_angular_velocity - tempCalc.a_cross_n[0] * a_angular_velocity;
    float jq2 = dvn + tempCalc.b_cross_n[1] * b_angular_velocity - tempCalc.a_cross_n[1] * a_angular_velocity;
    float det = tempCalc.k[0][0] * tempCalc.k[1][1] - tempCalc.k[0][1] * tempCalc.k[1][0];

    if (tempCalc.Manifold.restitutionBias[0] == FLT_MAX)
    {
        if (jq1 > 1.0f || jq2 > 1.0f) // Only apply restitution if the contact points are approaching fast enough
        {
            tempCalc.Manifold.restitutionBias[0] = std::max(tempCalc.a_Rigidbody.restitution, tempCalc.b_Rigidbody.restitution) * -jq1;
            tempCalc.Manifold.restitutionBias[1] = std::max(tempCalc.a_Rigidbody.restitution, tempCalc.b_Rigidbody.restitution) * -jq2;
        }
        else
        {
            tempCalc.Manifold.restitutionBias[0] = 0.0f;
            tempCalc.Manifold.restitutionBias[1] = 0.0f;
        }
    }

    if (std::abs(det) < 1e-8f)
    {
        // Contact points coincide -> K is singular and Inverse() would return
        // identity, producing a garbage impulse. Solve as a single contact.
        Solve1ContactConstraint(tempCalc);
        return;
    }
    const float BETA = 0.2f;     // 0.1–0.3 is the usual range
    const float SLOP = 0.01f;
    float posBias[2] = {
        BETA * std::max(tempCalc.Manifold.penetrationDepth[0] - SLOP, 0.0f) / tempCalc.DeltaTime,
        BETA * std::max(tempCalc.Manifold.penetrationDepth[1] - SLOP, 0.0f) / tempCalc.DeltaTime
    };
    float b1 = -jq1 + tempCalc.Manifold.restitutionBias[0] + posBias[0];
    float b2 = -jq2 + tempCalc.Manifold.restitutionBias[1] + posBias[1];

    Math::Matrix2x2 k_inverse = tempCalc.k.Inverse();
    Math::Vector2 impulse_on_a;
    Math::Vector2 impulse_on_b;
    Math::Vector2 lambda = k_inverse * Math::Vector2(b1, b2);

    CalculateImpulses(normal, tempCalc.k, b1, b2, impulse_on_a, impulse_on_b, lambda);
    tempCalc.impulses[0] += std::max(lambda.x, 0.0f);
    tempCalc.impulses[1] += std::max(lambda.y, 0.0f);

    if (invMassA > 0.0f)
    {
        float invInertiaA = tempCalc.a_Rigidbody.GetInverseInertia();
        tempCalc.a_Rigidbody.linearVelocity += impulse_on_a * invMassA;
        tempCalc.a_Rigidbody.angularVelocity -= tempCalc.a_cross_n[0] * lambda.x * invInertiaA;
        tempCalc.a_Rigidbody.angularVelocity -= tempCalc.a_cross_n[1] * lambda.y * invInertiaA;
    }
    if (invMassB > 0.0f)
    {
        float invInertiaB = tempCalc.b_Rigidbody.GetInverseInertia();
        tempCalc.b_Rigidbody.linearVelocity += impulse_on_b * invMassB;
        tempCalc.b_Rigidbody.angularVelocity += tempCalc.b_cross_n[0] * lambda.x * invInertiaB;
        tempCalc.b_Rigidbody.angularVelocity += tempCalc.b_cross_n[1] * lambda.y * invInertiaB;
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
    const float BETA = 0.2f;     // 0.1–0.3 is the usual range
    const float SLOP = 0.01f;
    float posBias = BETA * std::max(tempCalc.Manifold.penetrationDepth[0] - SLOP, 0.0f) / tempCalc.DeltaTime;
    float aCN = tempCalc.a_cross_n[0];
    float bCN = tempCalc.b_cross_n[0];
    float denominator = invMassA + invMassB +
        aCN * aCN * tempCalc.a_Rigidbody.GetInverseInertia() +
            bCN * bCN * tempCalc.b_Rigidbody.GetInverseInertia();
    float jq = (b_velocity - a_velocity).Dot(normal) + bCN * b_angular_velocity - aCN * a_angular_velocity;

    //calculating restitution bias if it's the first round:
    if (tempCalc.Manifold.restitutionBias[0] == FLT_MAX)
    {
        if (jq > 1.0f) // Only apply restitution if the contact point is approaching fast enough
        {
            tempCalc.Manifold.restitutionBias[0] = std::max(tempCalc.a_Rigidbody.restitution, tempCalc.b_Rigidbody.restitution) * -jq;
        }
        else
        {
            tempCalc.Manifold.restitutionBias[0] = 0.0f;
        }
    }

    float lambda = std::max((-jq + tempCalc.Manifold.restitutionBias[0] + posBias) / denominator,0.f);

    Math::Vector2 impulse_on_a = -normal * lambda;
    Math::Vector2 impulse_on_b = normal * lambda;
    tempCalc.impulses[0] = lambda;

    if (invMassA > 0.0f)
    {
        tempCalc.a_Rigidbody.linearVelocity += impulse_on_a * invMassA;
        tempCalc.a_Rigidbody.angularVelocity -= aCN * lambda * tempCalc.a_Rigidbody.GetInverseInertia();
    }
    if (invMassB > 0.0f)
    {
        tempCalc.b_Rigidbody.linearVelocity += impulse_on_b * invMassB;
        tempCalc.b_Rigidbody.angularVelocity += bCN * lambda * tempCalc.b_Rigidbody.GetInverseInertia();
    }
}
