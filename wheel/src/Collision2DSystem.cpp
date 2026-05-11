#include "systems/Collision2DSystem.h"

#include "EventBus.h"
#include "Events.h"
#include "components/Collider2D.h"
#include "components/Transform2D.h"
#include "helpers/Collision2DManifold.h"
#include "systems/subsystems/BoxBoxCollision2D.h"
#include "core/Scene.h"

void Wheel::Engine::Systems::Collision2DSystem::Update(float deltaTime)
{
    CheckNarrowPhase();
}

void Wheel::Engine::Systems::Collision2DSystem::CheckNarrowPhase()
{
    for (int i = 0; i < m_EntityIDs.size(); i++)
    {
        for (int j = i + 1; j < m_EntityIDs.size(); j++)
        {
            if (m_EntityIDs[i] == m_EntityIDs[j]) continue;
            auto& collider1 = m_Scene->GetComponent<Components::BoxCollider2D>(m_EntityIDs[i]);
            auto& transform1 = m_Scene->GetComponent<Components::Transform2D>(m_EntityIDs[i]);
            auto& collider2 = m_Scene->GetComponent<Components::BoxCollider2D>(m_EntityIDs[j]);
            auto& transform2 = m_Scene->GetComponent<Components::Transform2D>(m_EntityIDs[j]);
            Collision::Collision2DManifold manifold = Collision::BoxBoxCollision2D::BoxBoxCollision(collider1, transform1, collider2, transform2);
            if (manifold.isColliding)
            {
                Wheel::EventSystem::EventBus::Publish(Events::CollisionEnterEvent(manifold));
            }
        }
    }
}
