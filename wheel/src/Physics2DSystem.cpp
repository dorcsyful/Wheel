#include "../include/systems/Physics2DSystem.h"
#include "core/Scene.h"
#include "components/Transform2D.h"
#include "components/Collider2D.h"
#include "components/Rigidbody2D.h"
void Wheel::Engine::Systems::Physics2DSystem::Update(float deltaTime)
{
    if (!m_TransformPool || !m_ColliderPool || !m_RigidbodyPool) {
        m_TransformPool = m_Scene->GetComponentPool<Components::Transform2D>();
        m_ColliderPool  = m_Scene->GetComponentPool<Components::BoxCollider2D>();
        m_RigidbodyPool = m_Scene->GetComponentPool<Components::Rigidbody2D>();
    }

    for (auto id : m_EntityIDs)
    {
        auto& transform = m_TransformPool->GetComponent(id);
        auto& collider  = m_ColliderPool->GetComponent(id);
        auto& rigidbody = m_RigidbodyPool->GetComponent(id);

        if (transform.isDirty)
            collider.isDirty = true;
        if (rigidbody.rigidbodyType == Components::Rigidbody2DType::STATIC)
            continue;
        if (!rigidbody.active)
            continue;
        ApplyGravity(rigidbody, deltaTime);
        IntegrateVelocity(rigidbody, deltaTime);
        IntegratePosition(transform, rigidbody, deltaTime);
        rigidbody.ClearForces();
    }
}

void Wheel::Engine::Systems::Physics2DSystem::ApplyGravity(Components::Rigidbody2D& a_Rigidbody2D,float deltaTime)
{
        a_Rigidbody2D.AddForce(m_Gravity * a_Rigidbody2D.mass);
}

void Wheel::Engine::Systems::Physics2DSystem::IntegrateVelocity(Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{
    a_Rigidbody2D.linearVelocity = a_Rigidbody2D.linearVelocity +(a_Rigidbody2D.GetForce() / a_Rigidbody2D.mass) * deltaTime;
    a_Rigidbody2D.angularVelocity += (a_Rigidbody2D.GetTorque() / a_Rigidbody2D.inertia) * deltaTime;
    a_Rigidbody2D.linearVelocity *= (1.0f - a_Rigidbody2D.linearDamping * deltaTime);
    a_Rigidbody2D.angularVelocity *= (1.0f - a_Rigidbody2D.angularDamping * deltaTime);
}

void Wheel::Engine::Systems::Physics2DSystem::IntegratePosition(Components::Transform2D& a_Transform2D, Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{
    a_Transform2D.SetPosition(a_Transform2D.GetPosition() + a_Rigidbody2D.linearVelocity * deltaTime);
    a_Transform2D.SetRotation(a_Transform2D.GetRotation() + a_Rigidbody2D.angularVelocity * deltaTime);
}
