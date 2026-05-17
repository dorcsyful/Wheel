#include "../include/systems/subsystems/ConstraintSolver.h"

#include "components/Rigidbody2D.h"
#include "helpers/Collision2DManifold.h"

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

    Math::Matrix2x2 k;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            float aCrossN =(a_distance_from_com[i].Cross(normal) * (a_distance_from_com[j].Cross(normal)));
            float bCrossN =(b_distance_from_com[i].Cross(normal) * (b_distance_from_com[j].Cross(normal)));
            k[i][j] = invMassA + invMassB + aCrossN * a_A.GetInverseInertia() + bCrossN * a_B.GetInverseInertia();
        }
    }

    float jq1 = normal.x * b_velocity.x + normal.y * b_velocity.y + (b_distance_from_com[0].Cross(normal)) * b_angular_velocity - normal.x * a_velocity.x - normal.y * a_velocity.y - (a_distance_from_com[0].Cross(normal)) * a_angular_velocity;
    float jq2 = normal.x * b_velocity.x + normal.y * b_velocity.y + (b_distance_from_com[1].Cross(normal)) * b_angular_velocity - normal.x * a_velocity.x - normal.y * a_velocity.y - (a_distance_from_com[1].Cross(normal)) * a_angular_velocity;
    const float BAUMGARTE = 0.2f;
    const float SLOP = 0.01f;
    float bias1 = (BAUMGARTE / a_DeltaTime) * std::max(a_Manifold.penetrationDepth[0] - SLOP, 0.0f);
    float bias2 = (BAUMGARTE / a_DeltaTime) * std::max(a_Manifold.penetrationDepth[1] - SLOP, 0.0f);
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
    float b1 = (-jq1 + bias1);
    float b2 = (-jq2 + bias2);
    Math::Vector2 impulse_on_a;
    Math::Vector2 impulse_on_b;
    Math::Vector2 lambda = k_inverse * Math::Vector2(b1, b2);
    if (lambda[0] > 0 && lambda[1] > 0)
    {
        impulse_on_a = -normal * (lambda[0] + lambda[1]);
        impulse_on_b = normal * (lambda[0] + lambda[1]);
    }
    else if (lambda[0] > 0 && lambda[1] <= 0)
    {
        lambda[0] = b1 /k[0][0]; // No impulse at contact point 2
        lambda[1] = 0.0f;
        impulse_on_a = -normal * (lambda[0] + lambda[1]);
        impulse_on_b = normal * (lambda[0] + lambda[1]);

    }
    else if (lambda[0] <= 0 && lambda[1] > 0)
    {
        lambda[1] = b2/k[1][1]; // No impulse at contact point 1
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
        const float BAUMGARTE = 0.2f;
        const float SLOP = 0.01f;
        float bias = (BAUMGARTE / a_DeltaTime) * std::max(a_PenetrationDepth - SLOP, 0.0f);
        float lambda = std::max((-jq + bias) / denominator,0.f);

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
