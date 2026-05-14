#include "systems/Collision2DSystem.h"

#include "EventBus.h"
#include "Events.h"
#include "components/Collider2D.h"
#include "components/Transform2D.h"
#include "helpers/Collision2DManifold.h"
#include "systems/subsystems/BoxBoxCollision2D.h"
#include "core/Scene.h"
#ifdef DEBUG_BUILD
#include "../renderer/include/DebugDrawQueue.h"
#endif

void Wheel::Engine::Systems::Collision2DSystem::Update(float deltaTime)
{
    CheckNarrowPhase();
}

void Wheel::Engine::Systems::Collision2DSystem::CheckNarrowPhase()
{
    //Checking if collision vertices need to recompute
    for (auto id : m_EntityIDs)
    {
        auto& transform  = m_Scene->GetComponent<Components::Transform2D>(id);
        auto& collision = m_Scene->GetComponent<Components::BoxCollider2D>(id);
        if (transform.isDirty)
            collision.isDirty = true;
    }

    for (int i = 0; i < (int)m_EntityIDs.size(); i++)
    {
        for (int j = i + 1; j < (int)m_EntityIDs.size(); j++)
        {
            if (m_EntityIDs[i] == m_EntityIDs[j]) continue;
            auto& collider1  = m_Scene->GetComponent<Components::BoxCollider2D>(m_EntityIDs[i]);
            auto& transform1 = m_Scene->GetComponent<Components::Transform2D>(m_EntityIDs[i]);
            auto& collider2  = m_Scene->GetComponent<Components::BoxCollider2D>(m_EntityIDs[j]);
            auto& transform2 = m_Scene->GetComponent<Components::Transform2D>(m_EntityIDs[j]);
            Collision::Collision2DManifold manifold = Collision::BoxBoxCollision2D::BoxBoxCollision(collider1, transform1, collider2, transform2);
            if (manifold.isColliding)
            {
                Wheel::EventSystem::EventBus::Publish(Events::CollisionEnterEvent(manifold));
            }
        }
    }

    // Clear transform dirty flags — col.isDirty was already reset by GetVertices after recomputing
    for (auto id : m_EntityIDs)
        m_Scene->GetComponent<Components::Transform2D>(id).isDirty = false;
}