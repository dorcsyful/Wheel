#include "../include/systems/subsystems/ConstraintSolver.h"

#include <algorithm>

#include "components/Rigidbody2D.h"
#include "helpers/Collision2DManifold.h"

void Wheel::Engine::Physics::ConstraintSolver::WarmStart(TempCalculations& tempCalc, const float* a_NormalImpulse, const float* a_FrictionImpulse)
{
    Math::Vector2 normal  = tempCalc.Manifold.collisionNormal;
    Math::Vector2 tangent = tempCalc.Manifold.tangentNormal;
    float invMassA = tempCalc.a_Rigidbody.GetInverseMass();
    float invMassB = tempCalc.b_Rigidbody.GetInverseMass();
    float invInertiaA = tempCalc.a_Rigidbody.GetInverseInertia();
    float invInertiaB = tempCalc.b_Rigidbody.GetInverseInertia();

    for (int i = 0; i < tempCalc.Manifold.contactCount; i++)
    {
        float pn = a_NormalImpulse[i];
        float pt = a_FrictionImpulse[i];

        tempCalc.impulses[i] = pn;
        tempCalc.frictionImpulses[i] = pt;

        Math::Vector2 P = normal * pn + tangent * pt;
        tempCalc.a_Rigidbody.linearVelocity  -= P * invMassA;
        tempCalc.a_Rigidbody.angularVelocity -= tempCalc.a_distance_from_com[i].Cross(P) * invInertiaA;
        tempCalc.b_Rigidbody.linearVelocity  += P * invMassB;
        tempCalc.b_Rigidbody.angularVelocity += tempCalc.b_distance_from_com[i].Cross(P) * invInertiaB;
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
    float lambda = -jq1 / k;

    float max_force = std::sqrt(a_TempCalc.a_Rigidbody.friction * a_TempCalc.b_Rigidbody.friction) * a_TempCalc.impulses[i];
    float oldImpulse = a_TempCalc.frictionImpulses[i];
    float newImpulse = std::clamp(oldImpulse + lambda, -max_force, max_force);
    lambda = newImpulse - oldImpulse;
    a_TempCalc.frictionImpulses[i] = newImpulse;

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
    // don't fight sub-millimetre jitter every frame. Subtracting the current
    // relative pseudo velocity makes this safe to run every iteration: a contact
    // already corrected by a neighbour contributes nothing more.
    for (int i = 0; i < tempCalc.Manifold.contactCount; i++)
    {
        const float SLOP = 0.01f;
        const float MAX_LINEAR_CORRECTION = 0.2f;
        float corr = std::min(std::max(tempCalc.Manifold.penetrationDepth[i] - SLOP, 0.0f),
                              MAX_LINEAR_CORRECTION);
        float biasVel = corr / tempCalc.DeltaTime;

        float invMassA = tempCalc.a_Rigidbody.GetInverseMass();
        float invMassB = tempCalc.b_Rigidbody.GetInverseMass();
        Math::Vector2 normal = tempCalc.Manifold.collisionNormal;

        float pseudo_dvn =
                (tempCalc.b_Rigidbody.pseudoLinearVelocity - tempCalc.a_Rigidbody.pseudoLinearVelocity).Dot(normal)
              + tempCalc.b_cross_n[i] * tempCalc.b_Rigidbody.pseudoAngularVelocity
              - tempCalc.a_cross_n[i] * tempCalc.a_Rigidbody.pseudoAngularVelocity;

        // Only the remaining violation needs correction; clamp so we never pull apart.
        float lambda = std::max((biasVel - pseudo_dvn) / tempCalc.k[i][i], 0.0f);

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
        if (jq1 < 1.0f || jq2 < 1.0f) // Only apply restitution if the contact points are approaching fast enough
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
    Math::Matrix2x2 K = tempCalc.k;
    Math::Matrix2x2 k_inverse = K.Inverse();

    Math::Vector2 oldImpulse(tempCalc.impulses[0], tempCalc.impulses[1]);

    Math::Vector2 b(jq1 - tempCalc.Manifold.restitutionBias[0],
                    jq2 - tempCalc.Manifold.restitutionBias[1]);
    b = b - K * oldImpulse;

    Math::Vector2 x;
    // Case 1: both contacts pushing.
    x = (k_inverse * b) * -1.0f;
    if (x.x < 0.0f || x.y < 0.0f)
    {
        // Case 2: only contact 0 (force contact 1 to zero).
        x.x = -b.x / K[0][0];
        x.y = 0.0f;
        float vn2 = K[1][0] * x.x + b.y;
        if (x.x < 0.0f || vn2 < 0.0f)
        {
            // Case 3: only contact 1 (force contact 0 to zero).
            x.x = 0.0f;
            x.y = -b.y / K[1][1];
            float vn1 = K[0][1] * x.y + b.x;
            if (x.y < 0.0f || vn1 < 0.0f)
            {
                // Case 4: both contacts separating -> no normal impulse.
                x.x = 0.0f;
                x.y = 0.0f;
            }
        }
    }

    // Apply only the change relative to the accumulated impulse, and store the new
    // total for friction clamping and next-frame warm starting.
    Math::Vector2 d = x - oldImpulse;
    tempCalc.impulses[0] = x.x;
    tempCalc.impulses[1] = x.y;

    Math::Vector2 P = normal * (d.x + d.y);
    if (invMassA > 0.0f)
    {
        float invInertiaA = tempCalc.a_Rigidbody.GetInverseInertia();
        tempCalc.a_Rigidbody.linearVelocity  -= P * invMassA;
        tempCalc.a_Rigidbody.angularVelocity -= (tempCalc.a_cross_n[0] * d.x + tempCalc.a_cross_n[1] * d.y) * invInertiaA;
    }
    if (invMassB > 0.0f)
    {
        float invInertiaB = tempCalc.b_Rigidbody.GetInverseInertia();
        tempCalc.b_Rigidbody.linearVelocity  += P * invMassB;
        tempCalc.b_Rigidbody.angularVelocity += (tempCalc.b_cross_n[0] * d.x + tempCalc.b_cross_n[1] * d.y) * invInertiaB;
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
    float aCN = tempCalc.a_cross_n[0];
    float bCN = tempCalc.b_cross_n[0];
    float denominator = invMassA + invMassB +
        aCN * aCN * tempCalc.a_Rigidbody.GetInverseInertia() +
            bCN * bCN * tempCalc.b_Rigidbody.GetInverseInertia();
    float jq = (b_velocity - a_velocity).Dot(normal) + bCN * b_angular_velocity - aCN * a_angular_velocity;

    //calculating restitution bias if it's the first round:
    if (tempCalc.Manifold.restitutionBias[0] == FLT_MAX)
    {
        if (jq < 1.0f) // Only apply restitution if the contact point is approaching fast enough
        {
            tempCalc.Manifold.restitutionBias[0] = std::max(tempCalc.a_Rigidbody.restitution, tempCalc.b_Rigidbody.restitution) * -jq;
        }
        else
        {
            tempCalc.Manifold.restitutionBias[0] = 0.0f;
        }
    }

    // Unclamped impulse increment toward the target velocity.
    float lambda = (-jq + tempCalc.Manifold.restitutionBias[0]) / denominator;
    float oldImpulse = tempCalc.impulses[0];
    float newImpulse = std::max(oldImpulse + lambda, 0.0f);
    lambda = newImpulse - oldImpulse;
    tempCalc.impulses[0] = newImpulse;

    Math::Vector2 impulse_on_a = -normal * lambda;
    Math::Vector2 impulse_on_b = normal * lambda;

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