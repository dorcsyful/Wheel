#include "Physics2DSystem.h"
#include "core/Scene.h"
#include "common/Transform2D.h"
#include "../collision/Collider2D.h"
#include "Rigidbody2D.h"
#include "../collision/Collision2DSystem.h"
#include "CollisionConstraintSolver.h"



bool Wheel::Physics::Physics2DSystem::UpdateRigidbodyInertia(std::vector<unsigned>::value_type id, Common::Transform2D& transform, Rigidbody2D& rigidbody)
{
    const Core::Description& desc = m_DescriptionRef->at(id);
    if (desc.HasComponentType(m_BoxColliderDesc))
    {
        auto& collider = m_BoxColliderPool->GetComponent(id);
        if (transform.isDirty || collider.isDirty || rigidbody.isDirty)
        {
            rigidbody.SetInertia(CalculateBoxInertia(collider, transform, rigidbody.GetMass()));
            rigidbody.isDirty = false;
            transform.isDirty = false;
            collider.isDirty = false;            }
    }
    else if (desc.HasComponentType(m_CircleColliderDesc))
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

void Wheel::Physics::Physics2DSystem::EnsurePools()
{
    if (!m_TransformPool || !m_BoxColliderPool || !m_CircleColliderPool || !m_RigidbodyPool) {
        m_TransformPool = m_Scene->GetComponentPool<Common::Transform2D>();
        m_BoxColliderPool  = m_Scene->GetComponentPool<Collision::BoxCollider2D>();
        m_CircleColliderPool  = m_Scene->GetComponentPool<Collision::CircleCollider2D>();
        m_RigidbodyPool = m_Scene->GetComponentPool<Rigidbody2D>();
        m_DescriptionRef = m_Scene->GetEntityDescriptions();
        m_BoxColliderDesc = m_BoxColliderPool->GetDescription();
        m_CircleColliderDesc = m_CircleColliderPool->GetDescription();
    }
}

void Wheel::Physics::Physics2DSystem::Update(float deltaTime)
{
    EnsurePools();

    for (auto id : m_EntityIDs)
    {
        auto& transform = m_TransformPool->GetComponent(id);
        auto& rigidbody = m_RigidbodyPool->GetComponent(id);

        if (UpdateRigidbodyInertia(id, transform, rigidbody)) continue;

        if (rigidbody.GetType() == Rigidbody2DType::STATIC || rigidbody.GetType() == Rigidbody2DType::KINEMATIC)
            continue;
        if (!rigidbody.active)
            continue;

        if (rigidbody.affectedByGravity) ApplyGravity(rigidbody, deltaTime);
        IntegrateVelocity(rigidbody, deltaTime);
        rigidbody.ClearForces();

    }

    m_CollisionResolver->SolveConstraints(m_Scene->GetSystem<Collision::Collision2DSystem>()->GetManifolds(), m_Scene, deltaTime);
    for (uint32_t id : m_EntityIDs)
    {
        Rigidbody2D& rigidbody2D = m_RigidbodyPool->GetComponent(id);
        if (!rigidbody2D.active || rigidbody2D.GetType() != Rigidbody2DType::DYNAMIC
            )
            continue;
        m_CollisionResolver->IntegratePseudoPosition(m_TransformPool->GetComponent(id), rigidbody2D, deltaTime);
        IntegratePosition(m_TransformPool->GetComponent(id), rigidbody2D, deltaTime);
    }

}

void Wheel::Physics::Physics2DSystem::ApplyGravity(Rigidbody2D& a_Rigidbody2D,float deltaTime)
{
        a_Rigidbody2D.AddForce(m_Gravity * a_Rigidbody2D.GetMass());
}

void Wheel::Physics::Physics2DSystem::IntegrateVelocity(Rigidbody2D& a_Rigidbody2D, float deltaTime)
{
    a_Rigidbody2D.linearVelocity = a_Rigidbody2D.linearVelocity +(a_Rigidbody2D.GetForce() * a_Rigidbody2D.GetInverseMass()) * deltaTime;
    a_Rigidbody2D.angularVelocity += (a_Rigidbody2D.GetTorque() * a_Rigidbody2D.GetInverseInertia()) * deltaTime;
    a_Rigidbody2D.linearVelocity *= (1.0f - a_Rigidbody2D.linearDamping * deltaTime);
    a_Rigidbody2D.angularVelocity *= (1.0f - a_Rigidbody2D.angularDamping * deltaTime);
}
void Wheel::Physics::Physics2DSystem::IntegratePosition(Common::Transform2D& a_Transform2D, Rigidbody2D& a_Rigidbody2D, float deltaTime)
{

    a_Transform2D.SetPosition(a_Transform2D.GetPosition() + a_Rigidbody2D.linearVelocity * deltaTime);
    a_Transform2D.SetRotationInRadians(a_Transform2D.GetRotationInRadians() + a_Rigidbody2D.angularVelocity * deltaTime);
}

float Wheel::Physics::Physics2DSystem::CalculateBoxInertia(const Collision::BoxCollider2D& a_Collider,
    const Common::Transform2D& a_Transform, float mass)
{
    float width = a_Collider.GetWidth() * a_Transform.GetScale().x;
    float height = a_Collider.GetHeight() * a_Transform.GetScale().y;
    return (1.f/12.f) * mass * (std::pow(width,2) + std::pow(height,2));
}

float Wheel::Physics::Physics2DSystem::CalculateCircleInertia(const Collision::CircleCollider2D& a_Collider,
    const Common::Transform2D& a_Transform, float mass)
{
    // Solid disk: I = 1/2 * m * r^2 (uniform scale applied to the radius).
    float radius = a_Collider.radius * a_Transform.GetScale().x;
    return 0.5f * mass * radius * radius;
}

