#include "systems/Collision2DSystem.h"

#include "EventBus.h"
#include "Events.h"
#include "components/Collider2D.h"
#include "components/Transform2D.h"
#include "helpers/Collision2DManifold.h"
#include "../include/systems/helpers/BoxBoxCollision2D.h"
#include "../include/systems/helpers/CircleBoxCollision2D.h"
#include "../include/systems/helpers/CircleCircleCollision2D.h"
#include "core/Scene.h"


void Wheel::Engine::Systems::Collision2DSystem::Update(float deltaTime)
{
    CheckNarrowPhase();
}

void Wheel::Engine::Systems::Collision2DSystem::SetDirtyFlag(Wheel::Engine::Description boxComponentDesc, Wheel::Engine::Description circleComponentDesc, bool enable)
{

    for (auto id : m_EntityIDs)
    {
        auto& transform = m_TransformPool->GetComponent(id);
        if (m_DescriptionRef->at(id).HasComponentType(boxComponentDesc))
        {
            auto& collision = m_BoxColliderPool->GetComponent(id);
            if (transform.isDirty)
                collision.isDirty = enable;
        }
        else
        {
            if (m_DescriptionRef->at(id).HasComponentType(circleComponentDesc))
            {
                auto& collision = m_CircleColliderPool->GetComponent(id);
                if (transform.isDirty)
                    collision.isDirty = enable;
            }
        }
    }
}

void Wheel::Engine::Systems::Collision2DSystem::CheckNarrowPhase()
{
    m_Manifolds.clear();
    if (!m_BoxColliderPool || !m_TransformPool || !m_CircleColliderPool || !m_DescriptionRef) {
        m_BoxColliderPool  = m_Scene->GetComponentPool<Components::BoxCollider2D>();
        m_CircleColliderPool  = m_Scene->GetComponentPool<Components::CircleCollider2D>();
        m_TransformPool = m_Scene->GetComponentPool<Components::Transform2D>();
        m_DescriptionRef = m_Scene->GetEntityDescriptions();
    }

    Description boxComponentDesc = m_BoxColliderPool->GetDescription();
    Description circleComponentDesc = m_CircleColliderPool->GetDescription();
    bool enable = true;
    SetDirtyFlag(boxComponentDesc, circleComponentDesc, enable);

    for (int i = 0; i < (int)m_EntityIDs.size(); i++)
    {
        Description desc1 = m_DescriptionRef->at(m_EntityIDs[i]);
        auto& transform1 = m_TransformPool->GetComponent(m_EntityIDs[i]);
        bool desc1HasBoxComponent = desc1.HasComponentType(boxComponentDesc);
        bool desc1HasCircleComponent = desc1.HasComponentType(circleComponentDesc);
        for (int j = i + 1; j < (int)m_EntityIDs.size(); j++)
        {
            Collision::Collision2DManifold manifold = Collision::Collision2DManifold();
            Description desc2 = m_DescriptionRef->at(m_EntityIDs[j]);
            auto& transform2 = m_TransformPool->GetComponent(m_EntityIDs[j]);
            bool desc2HasBoxComponent = desc2.HasComponentType(boxComponentDesc);
            bool desc2HasCircleComponent = desc2.HasComponentType(circleComponentDesc);
            if (desc1HasBoxComponent && desc2HasBoxComponent)
            {
                auto& collider1  = m_BoxColliderPool->GetComponent(m_EntityIDs[i]);
                auto& collider2  = m_BoxColliderPool->GetComponent(m_EntityIDs[j]);
                manifold = Collision::BoxBoxCollision2D::BoxBoxCollision(collider1, transform1, collider2, transform2);
            }
            else if (desc1HasBoxComponent && desc2HasCircleComponent)
            {
                // i is the box, j is the circle.
                auto& boxCollider    = m_BoxColliderPool->GetComponent(m_EntityIDs[i]);
                auto& circleCollider = m_CircleColliderPool->GetComponent(m_EntityIDs[j]);
                Collision::BoxBoxCollision2D::GetVertices(boxCollider, transform1); // ensure cachedVertices are current
                manifold = Collision::CircleBoxCollision2D::CheckCircleBoxCollision(transform2, circleCollider, transform1, boxCollider);
            }
            else if (desc2HasBoxComponent && desc1HasCircleComponent)
            {
                // i is the circle, j is the box.
                auto& circleCollider = m_CircleColliderPool->GetComponent(m_EntityIDs[i]);
                auto& boxCollider    = m_BoxColliderPool->GetComponent(m_EntityIDs[j]);
                Collision::BoxBoxCollision2D::GetVertices(boxCollider, transform2); // ensure cachedVertices are current
                manifold = Collision::CircleBoxCollision2D::CheckCircleBoxCollision(transform1, circleCollider, transform2, boxCollider);
            }
            else if (desc1.HasComponentType(circleComponentDesc) && desc2.HasComponentType(circleComponentDesc))
            {
                auto& collider1  = m_CircleColliderPool->GetComponent(m_EntityIDs[i]);
                auto& collider2  = m_CircleColliderPool->GetComponent(m_EntityIDs[j]);
                manifold = Physics::CircleCircleCollision2D::CheckCircleCircleCollision(transform1, transform2, collider1, collider2);
            }


            if (manifold.isColliding)
            {
                m_Manifolds.push_back(manifold);
            }
        }
    }
    EventSystem::EventBus::Publish(Events::CollisionResultsFinished(&m_Manifolds));
    enable = false;
    SetDirtyFlag(boxComponentDesc, circleComponentDesc, enable);
}
