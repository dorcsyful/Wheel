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

void Wheel::Engine::Physics::ConstraintSolver::SolvePseudoVelocities(const Collision::Collision2DManifold& a_Manifold,
    float a_DeltaTime, const Components::Transform2D& a_ATransform, const Components::Transform2D& a_BTransform,
    Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B)
{
    //(vA + ωA × rA - vB - ωB × rB)
    Math::Matrix2x2 R_a = a_ATransform.GetRotationMatrix();
    Math::Vector2 a_distance_from_com[2] = {
        (a_Manifold.contactPoint[0] - a_ATransform.GetPosition()) - R_a * a_A.centerOfMass,
        (a_Manifold.contactPoint[1] - a_ATransform.GetPosition()) - R_a * a_A.centerOfMass
    };
    Math::Matrix2x2 R_b = a_BTransform.GetRotationMatrix();
    Math::Vector2 b_distance_from_com[2] = {
        (a_Manifold.contactPoint[0] - a_BTransform.GetPosition()) - R_b * a_B.centerOfMass,
        (a_Manifold.contactPoint[1] - a_BTransform.GetPosition()) - R_b * a_B.centerOfMass
    };
    float invMassA = a_A.GetInverseMass();
    float invMassB = a_B.GetInverseMass();
    Math::Vector2 normal = a_Manifold.collisionNormal;

    Math::Matrix2x2 k;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            //How much a unit impulse at contact point j affects the relative velocity at contact point i
            float aCrossN =(a_distance_from_com[i].Cross(normal) * (a_distance_from_com[j].Cross(normal)));
            float bCrossN =(b_distance_from_com[i].Cross(normal) * (b_distance_from_com[j].Cross(normal)));
            k[i][j] = invMassA + invMassB + aCrossN * a_A.GetInverseInertia() + bCrossN * a_B.GetInverseInertia();
        }
    }
    // Position-only solve: the pseudo correction depends solely on the
    // geometric overlap, never on real velocity (the velocity solver owns
    // that). SLOP leaves a small allowed penetration so resting bodies
    // don't fight sub-millimetre jitter every frame.
    const float SLOP = 0.01f;
    for (int i = 0; i < a_Manifold.contactCount; i++)
    {
        const float MAX_LINEAR_CORRECTION = 0.2f;
        float corr = std::min(std::max(a_Manifold.penetrationDepth[i] - SLOP, 0.0f),
                              MAX_LINEAR_CORRECTION);
        float biasVel = corr / a_DeltaTime;
        float lambda  = biasVel / k[i][i];   // biasVel >= 0, so lambda >= 0

        // Same structure (and signs) as Solve1ContactConstraint, written
        // into the throwaway pseudo channel instead of real velocity.
        if (invMassA > 0.0f)
        {
            a_A.pseudoLinearVelocity  -= normal * (lambda * invMassA);
            a_A.pseudoAngularVelocity -= a_distance_from_com[i].Cross(normal) * lambda * a_A.GetInverseInertia();
        }
        if (invMassB > 0.0f)
        {
            a_B.pseudoLinearVelocity  += normal * (lambda * invMassB);
            a_B.pseudoAngularVelocity += b_distance_from_com[i].Cross(normal) * lambda * a_B.GetInverseInertia();
        }
    }
}

void Wheel::Engine::Physics::ConstraintSolver::Solve2ContactConstraint(const Collision::Collision2DManifold& a_Manifold, float a_DeltaTime,
                                                                       const Components::Transform2D& a_ATransform, const Components::Transform2D& a_BTransform,
                                                                       Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B)
{
    Math::Vector2 normal = a_Manifold.collisionNormal;

    Math::Vector2 a_velocity = a_A.linearVelocity;
    float a_angular_velocity = a_A.angularVelocity;

    Math::Vector2 b_velocity = a_B.linearVelocity;
    float b_angular_velocity = a_B.angularVelocity;

    Math::Matrix2x2 R_a = a_ATransform.GetRotationMatrix();
    Math::Vector2 a_distance_from_com[2] = {
        (a_Manifold.contactPoint[0] - a_ATransform.GetPosition()) - R_a * a_A.centerOfMass,
        (a_Manifold.contactPoint[1] - a_ATransform.GetPosition()) - R_a * a_A.centerOfMass
    };
    Math::Matrix2x2 R_b = a_BTransform.GetRotationMatrix();
    Math::Vector2 b_distance_from_com[2] = {
        (a_Manifold.contactPoint[0] - a_BTransform.GetPosition()) - R_b * a_B.centerOfMass,
        (a_Manifold.contactPoint[1] - a_BTransform.GetPosition()) - R_b * a_B.centerOfMass
    };
    float invMassA = a_A.GetInverseMass();
    float invMassB = a_B.GetInverseMass();
    //Build the effective mass matrix: K = J M⁻¹ Jᵀ
    Math::Matrix2x2 k;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            //How much a unit impulse at contact point j affect the relative velocity at contact point i
            float aCrossN =(a_distance_from_com[i].Cross(normal) * (a_distance_from_com[j].Cross(normal)));
            float bCrossN =(b_distance_from_com[i].Cross(normal) * (b_distance_from_com[j].Cross(normal)));
            k[i][j] = invMassA + invMassB + aCrossN * a_A.GetInverseInertia() + bCrossN * a_B.GetInverseInertia();
        }
    }
    //Velocities at the contact points
    float jq1 = normal.x * b_velocity.x + normal.y * b_velocity.y + (b_distance_from_com[0].Cross(normal)) * b_angular_velocity - normal.x * a_velocity.x - normal.y * a_velocity.y - (a_distance_from_com[0].Cross(normal)) * a_angular_velocity;
    float jq2 = normal.x * b_velocity.x + normal.y * b_velocity.y + (b_distance_from_com[1].Cross(normal)) * b_angular_velocity - normal.x * a_velocity.x - normal.y * a_velocity.y - (a_distance_from_com[1].Cross(normal)) * a_angular_velocity;
    float det = k[0][0] * k[1][1] - k[0][1] * k[1][0];
    if (std::abs(det) < 1e-8f)
    {
        // Contact points coincide -> K is singular and Inverse() would return
        // identity, producing a garbage impulse. Solve as a single contact.
        Solve1ContactConstraint(a_Manifold.contactPoint[0], normal,
            a_Manifold.penetrationDepth[0], a_DeltaTime,
            a_ATransform, a_BTransform, a_A, a_B);
        return;
    }

    Math::Matrix2x2 k_inverse = k.Inverse();
    float b1 = (-jq1);
    float b2 = (-jq2);
    Math::Vector2 impulse_on_a;
    Math::Vector2 impulse_on_b;
    Math::Vector2 lambda = k_inverse * Math::Vector2(b1, b2);

    CalculateImpulses(normal, k, b1, b2, impulse_on_a, impulse_on_b, lambda);

    if (invMassA > 0.0f)
    {
        a_A.linearVelocity += impulse_on_a * invMassA;
        a_A.angularVelocity -= a_distance_from_com[0].Cross(normal) * lambda.x * a_A.GetInverseInertia();
        a_A.angularVelocity -= a_distance_from_com[1].Cross(normal) * lambda.y * a_A.GetInverseInertia();
    }
    if (invMassB > 0.0f)
    {
        a_B.linearVelocity += impulse_on_b * invMassB;
        a_B.angularVelocity += b_distance_from_com[0].Cross(normal) * lambda.x * a_B.GetInverseInertia();
        a_B.angularVelocity += b_distance_from_com[1].Cross(normal) * lambda.y * a_B.GetInverseInertia();
    }
}

void Wheel::Engine::Physics::ConstraintSolver::Solve1ContactConstraint(const Math::Vector2& a_ContactPoint, const Math::Vector2& a_CollisionNormal,
                                                                       float a_PenetrationDepth, float a_DeltaTime,
                                                                       const Components::Transform2D& a_ATransform, const Components::Transform2D& a_BTransform, Components::Rigidbody2D& a_A, Components::Rigidbody2D& a_B)
{
        Math::Vector2 normal = a_CollisionNormal;

        Math::Vector2 a_velocity = a_A.linearVelocity;
        float a_angular_velocity = a_A.angularVelocity;

        Math::Vector2 b_velocity = a_B.linearVelocity;
        float b_angular_velocity = a_B.angularVelocity;

        Math::Matrix2x2 R_a = a_ATransform.GetRotationMatrix();
        Math::Vector2 a_distance_from_com = (a_ContactPoint - a_ATransform.GetPosition()) - R_a * a_A.centerOfMass;
        Math::Matrix2x2 R_b = a_BTransform.GetRotationMatrix();

        Math::Vector2 b_distance_from_com = (a_ContactPoint - a_BTransform.GetPosition()) - R_b * a_B.centerOfMass;

        float invMassA = a_A.GetInverseMass();
        float invMassB = a_B.GetInverseMass();

        float denominator = invMassA + invMassB +
            std::pow(a_distance_from_com.Cross(normal),2) * a_A.GetInverseInertia() +
                std::pow(b_distance_from_com.Cross(normal),2) * a_B.GetInverseInertia();
        float jq = normal.x * b_velocity.x + normal.y * b_velocity.y + (b_distance_from_com.Cross(normal)) * b_angular_velocity - normal.x * a_velocity.x - normal.y * a_velocity.y - (a_distance_from_com.Cross(normal)) * a_angular_velocity;
        float lambda = std::max((-jq) / denominator,0.f);

        Math::Vector2 impulse_on_a = -normal * lambda;
        Math::Vector2 impulse_on_b = normal * lambda;

        if (invMassA > 0.0f)
        {
            a_A.linearVelocity += impulse_on_a * invMassA;
            a_A.angularVelocity -= a_distance_from_com.Cross(normal) * lambda * a_A.GetInverseInertia();
        }
        if (invMassB > 0.0f)
        {
            a_B.linearVelocity += impulse_on_b * invMassB;
            a_B.angularVelocity += b_distance_from_com.Cross(normal) * lambda * a_B.GetInverseInertia();
        }

}
