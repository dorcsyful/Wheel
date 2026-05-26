#include "../include/systems/Physics2DSystem.h"
#include "core/Scene.h"
#include "components/Transform2D.h"
#include "components/Collider2D.h"
#include "components/Rigidbody2D.h"
#include "systems/Collision2DSystem.h"
#include "systems/subsystems/ConstraintSolver.h"

void Wheel::Engine::Systems::Physics2DSystem::Update(float deltaTime)
{
    if (!m_TransformPool || !m_ColliderPool || !m_RigidbodyPool) {
        m_TransformPool = m_Scene->GetComponentPool<Components::Transform2D>();
        m_ColliderPool  = m_Scene->GetComponentPool<Components::BoxCollider2D>();
        m_RigidbodyPool = m_Scene->GetComponentPool<Components::Rigidbody2D>();
    }
    if (!m_Manifolds) {
        m_Manifolds = m_Scene->GetSystem<Collision2DSystem>()->GetManifolds();
    }

    for (auto id : m_EntityIDs)
    {
        auto& transform = m_TransformPool->GetComponent(id);
        auto& collider  = m_ColliderPool->GetComponent(id);
        auto& rigidbody = m_RigidbodyPool->GetComponent(id);

        if (transform.isDirty || collider.isDirty || rigidbody.isDirty)
        {
            rigidbody.SetInertia(CalculateInertia(collider, transform, rigidbody.GetMass()));
            rigidbody.isDirty = false;
            transform.isDirty = false;
            collider.isDirty = false;
        }
        if (rigidbody.GetType() == Components::Rigidbody2DType::STATIC)
            continue;
        if (!rigidbody.active)
            continue;

        if (rigidbody.affectedByGravity) ApplyGravity(rigidbody, deltaTime);
        IntegrateVelocity(rigidbody, deltaTime);
        rigidbody.ClearForces();

    }

    SolveConstraints(deltaTime);
    for (uint32_t id : m_EntityIDs)
    {
        Components::Rigidbody2D& rigidbody2D = m_RigidbodyPool->GetComponent(id);
        if (!rigidbody2D.active || rigidbody2D.GetType() == Components::Rigidbody2DType::STATIC)
            continue;
        IntegratePseudoPosition(m_TransformPool->GetComponent(id), rigidbody2D, deltaTime);
        IntegratePosition(m_TransformPool->GetComponent(id), rigidbody2D, deltaTime);
    }

}

void Wheel::Engine::Systems::Physics2DSystem::ApplyGravity(Components::Rigidbody2D& a_Rigidbody2D,float deltaTime)
{
        a_Rigidbody2D.AddForce(m_Gravity * a_Rigidbody2D.GetMass());
}

void Wheel::Engine::Systems::Physics2DSystem::IntegrateVelocity(Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{
    a_Rigidbody2D.linearVelocity = a_Rigidbody2D.linearVelocity +(a_Rigidbody2D.GetForce() * a_Rigidbody2D.GetInverseMass()) * deltaTime;
    a_Rigidbody2D.angularVelocity += (a_Rigidbody2D.GetTorque() / a_Rigidbody2D.GetInertia()) * deltaTime;
    a_Rigidbody2D.linearVelocity *= (1.0f - a_Rigidbody2D.linearDamping * deltaTime);
    a_Rigidbody2D.angularVelocity *= (1.0f - a_Rigidbody2D.angularDamping * deltaTime);
}

void Wheel::Engine::Systems::Physics2DSystem::IntegratePseudoPosition(Components::Transform2D& a_Transform2D,
    Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{
    a_Transform2D.SetPosition(a_Transform2D.GetPosition() + a_Rigidbody2D.pseudoLinearVelocity * deltaTime);
    a_Transform2D.SetRotationInRadians(a_Transform2D.GetRotationInRadians() + a_Rigidbody2D.pseudoAngularVelocity * deltaTime);
    a_Rigidbody2D.pseudoLinearVelocity = Math::Vector2(0.0f, 0.0f);
    a_Rigidbody2D.pseudoAngularVelocity = 0.0f;

}

void Wheel::Engine::Systems::Physics2DSystem::IntegratePosition(Components::Transform2D& a_Transform2D, Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{

    a_Transform2D.SetPosition(a_Transform2D.GetPosition() + a_Rigidbody2D.linearVelocity * deltaTime);
    a_Transform2D.SetRotationInRadians(a_Transform2D.GetRotationInRadians() + a_Rigidbody2D.angularVelocity * deltaTime);
}

float Wheel::Engine::Systems::Physics2DSystem::CalculateInertia(const Components::BoxCollider2D& a_Collider,
    const Components::Transform2D& a_Transform, float mass)
{
    if (a_Collider.type == Components::E_COLLIDER2_D::BOX)
    {
        float width = a_Collider.GetWidth() * a_Transform.GetScale().x;
        float height = a_Collider.GetHeight() * a_Transform.GetScale().y;
        return (1.f/12.f) * mass * (std::pow(width,2) + std::pow(height,2));
    }
    return 0.0f;
}

void Wheel::Engine::Systems::Physics2DSystem::SolveConstraints(float a_DeltaTime)
{
    std::vector<Physics::TempCalculations> tempCalcs = {};
    //Collision constraints
    for (int i = 0; i < MAX_CONSTRAINT_ITERATION; i++)
    {
        size_t tcIdx = 0;
        for (int j = 0; j < m_Manifolds->size(); j++)
        {
            if (!m_Manifolds->at(j).isColliding)
                continue;
            if (i == 0)
            {
                tempCalcs.push_back(Physics::ConstraintSolver::PrepareConstraintSolver((*m_Manifolds)[j],
                m_TransformPool->GetComponent((*m_Manifolds)[j].collider1), m_TransformPool->GetComponent((*m_Manifolds)[j].collider2),
                m_RigidbodyPool->GetComponent((*m_Manifolds)[j].collider1), m_RigidbodyPool->GetComponent((*m_Manifolds)[j].collider2),
                a_DeltaTime));
                Physics::ConstraintSolver::SolvePseudoVelocities(tempCalcs.back());
            }
            auto& manifold = (*m_Manifolds)[j];
            if (manifold.contactCount == 1)
            {
                Engine::Physics::ConstraintSolver::Solve1ContactConstraint(tempCalcs[tcIdx]);
            }
            else if (manifold.contactCount == 2)
            {
                Engine::Physics::ConstraintSolver::Solve2ContactConstraint(tempCalcs[tcIdx]);
            }
            if (i + 1 == MAX_CONSTRAINT_ITERATION)
            {
                if (m_Manifolds->at(j).contactCount == 1) Physics::ConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],0);
                else
                {
                    Physics::ConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],0);
                    Physics::ConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],1);
                }
            }
            tcIdx++;

        }
    }
    for (auto& tc : tempCalcs)
    {
        int contactCount = tc.Manifold.contactCount;
        for (int i = 0; i < contactCount; i++)
            Physics::ConstraintSolver::SolveFrictionConstraint(tc, i);
    }
}
