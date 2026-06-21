#include "../include/systems/Physics2DSystem.h"
#include "core/Scene.h"
#include "components/Transform2D.h"
#include "components/Collider2D.h"
#include "components/Rigidbody2D.h"
#include "systems/Collision2DSystem.h"
#include "../include/systems/helpers/CollisionConstraintSolver.h"



bool Wheel::Engine::Systems::Physics2DSystem::UpdateRigidbodyInertia(std::vector<unsigned>::value_type id, Wheel::Components::Transform2D& transform, Wheel::Components::Rigidbody2D& rigidbody)
{
    if (m_Scene->HasComponent<Components::BoxCollider2D>(id))
    {
        auto& collider = m_BoxColliderPool->GetComponent(id);
        if (transform.isDirty || collider.isDirty || rigidbody.isDirty)
        {
            rigidbody.SetInertia(CalculateBoxInertia(collider, transform, rigidbody.GetMass()));
            rigidbody.isDirty = false;
            transform.isDirty = false;
            collider.isDirty = false;            }
    }
    else if (m_Scene->HasComponent<Components::CircleCollider2D>(id))
    {
        auto& collider = m_CircleColliderPool->GetComponent(id);
        rigidbody.SetInertia(CalculateCircleInertia(collider, transform, rigidbody.GetMass()));
        rigidbody.isDirty = false;
        transform.isDirty = false;
        collider.isDirty = false;
    }
    else
    {
        return true; //No collider, skip
    }
    return false;
}

void Wheel::Engine::Systems::Physics2DSystem::EnsurePools()
{
    if (!m_TransformPool || !m_BoxColliderPool || !m_CircleColliderPool || !m_RigidbodyPool) {
        m_TransformPool = m_Scene->GetComponentPool<Components::Transform2D>();
        m_BoxColliderPool  = m_Scene->GetComponentPool<Components::BoxCollider2D>();
        m_CircleColliderPool  = m_Scene->GetComponentPool<Components::CircleCollider2D>();
        m_RigidbodyPool = m_Scene->GetComponentPool<Components::Rigidbody2D>();
    }
}

void Wheel::Engine::Systems::Physics2DSystem::Update(float deltaTime)
{
    EnsurePools();

    for (auto id : m_EntityIDs)
    {
        auto& transform = m_TransformPool->GetComponent(id);
        auto& rigidbody = m_RigidbodyPool->GetComponent(id);

        if (UpdateRigidbodyInertia(id, transform, rigidbody)) continue;

        if (rigidbody.GetType() == Components::Rigidbody2DType::STATIC)
            continue;
        if (!rigidbody.active)
            continue;

        if (rigidbody.affectedByGravity) ApplyGravity(rigidbody, deltaTime);
        IntegrateVelocity(rigidbody, deltaTime);
        rigidbody.ClearForces();

    }

    m_CollisionResolver->SolveConstraints(m_Scene->GetSystem<Collision2DSystem>()->GetManifolds(), m_Scene, deltaTime);
    for (uint32_t id : m_EntityIDs)
    {
        Components::Rigidbody2D& rigidbody2D = m_RigidbodyPool->GetComponent(id);
        if (!rigidbody2D.active || rigidbody2D.GetType() == Components::Rigidbody2DType::STATIC)
            continue;
        m_CollisionResolver->IntegratePseudoPosition(m_TransformPool->GetComponent(id), rigidbody2D, deltaTime);
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
    a_Rigidbody2D.angularVelocity += (a_Rigidbody2D.GetTorque() * a_Rigidbody2D.GetInverseInertia()) * deltaTime;
    a_Rigidbody2D.linearVelocity *= (1.0f - a_Rigidbody2D.linearDamping * deltaTime);
    a_Rigidbody2D.angularVelocity *= (1.0f - a_Rigidbody2D.angularDamping * deltaTime);
}
void Wheel::Engine::Systems::Physics2DSystem::IntegratePosition(Components::Transform2D& a_Transform2D, Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{

    a_Transform2D.SetPosition(a_Transform2D.GetPosition() + a_Rigidbody2D.linearVelocity * deltaTime);
    a_Transform2D.SetRotationInRadians(a_Transform2D.GetRotationInRadians() + a_Rigidbody2D.angularVelocity * deltaTime);
}

float Wheel::Engine::Systems::Physics2DSystem::CalculateBoxInertia(const Components::BoxCollider2D& a_Collider,
    const Components::Transform2D& a_Transform, float mass)
{
    float width = a_Collider.GetWidth() * a_Transform.GetScale().x;
    float height = a_Collider.GetHeight() * a_Transform.GetScale().y;
    return (1.f/12.f) * mass * (std::pow(width,2) + std::pow(height,2));
}

float Wheel::Engine::Systems::Physics2DSystem::CalculateCircleInertia(const Components::CircleCollider2D& a_Collider,
    const Components::Transform2D& a_Transform, float mass)
{
    // Solid disk: I = 1/2 * m * r^2 (uniform scale applied to the radius).
    float radius = a_Collider.radius * a_Transform.GetScale().x;
    return 0.5f * mass * radius * radius;
}

