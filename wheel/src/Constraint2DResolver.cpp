#include "systems/subsystems/Constraint2DResolver.h"
#include "core/Scene.h"
#include "helpers/Collision2DManifold.h"
#include "systems/helpers/CollisionConstraintSolver.h"
namespace Wheel::Engine::Physics
{
    // Order-independent key for a colliding pair
    uint64_t MakeContactKey(uint32_t a_First, uint32_t a_Second)
    {
        uint32_t lo = a_First < a_Second ? a_First : a_Second;
        uint32_t hi = a_First < a_Second ? a_Second : a_First;
        return (static_cast<uint64_t>(hi) << 32) | static_cast<uint64_t>(lo);
    }
}

Wheel::Engine::Subsystems::Constraint2DResolver::Constraint2DResolver()
{
    EventSystem::EventBus::Subscribe<Events::ComponentAddedEvent<Components::DistanceJoint2D>>(
        [this](const Events::ComponentAddedEvent<Components::DistanceJoint2D>& a_Event)
        {
            m_JointEntities.insert(a_Event.entityId);
        }, m_Tokens);

    EventSystem::EventBus::Subscribe<Events::ComponentRemovedEvent<Components::DistanceJoint2D>>(
        [this](const Events::ComponentRemovedEvent<Components::DistanceJoint2D>& a_Event)
        {
            m_JointEntities.erase(a_Event.entityId);
        }, m_Tokens);

    // Entity destruction tears the joint down without firing ComponentRemovedEvent,
    // so catch that path too.
    EventSystem::EventBus::Subscribe<Events::EntityDestroyed>(
        [this](const Events::EntityDestroyed& a_Event)
        {
            m_JointEntities.erase(a_Event.entityId);
        }, m_Tokens);
}
void Wheel::Engine::Subsystems::Constraint2DResolver::IntegratePseudoPosition(Components::Transform2D& a_Transform2D,
    Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{
    a_Transform2D.SetPosition(a_Transform2D.GetPosition() + a_Rigidbody2D.pseudoLinearVelocity * deltaTime);
    a_Transform2D.SetRotationInRadians(a_Transform2D.GetRotationInRadians() + a_Rigidbody2D.pseudoAngularVelocity * deltaTime);
    a_Rigidbody2D.pseudoLinearVelocity = Math::Vector2(0.0f, 0.0f);
    a_Rigidbody2D.pseudoAngularVelocity = 0.0f;

}

void Wheel::Engine::Subsystems::Constraint2DResolver::WarmStartCollision(std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs, size_t tcIdx, int j)
{
    Collision::Collision2DManifold& wsManifold = (*m_Manifolds)[j];
    float startNormal[2]   = { 0.0f, 0.0f };
    float startFriction[2] = { 0.0f, 0.0f };
    auto cacheIt = m_ContactCache.find(Wheel::Engine::Physics::MakeContactKey(wsManifold.collider1, wsManifold.collider2));
    if (cacheIt != m_ContactCache.end())
    {
        for (int c = 0; c < wsManifold.contactCount; c++)
            for (int p = 0; p < 2; p++)
                if (cacheIt->second.id[p] == wsManifold.contactId[c])
                {
                    startNormal[c]   = cacheIt->second.normalImpulse[p];
                    startFriction[c] = cacheIt->second.frictionImpulse[p];
                    break;
                }
    }
    Physics::CollisionConstraintSolver::WarmStart(tempCalcs[tcIdx], startNormal, startFriction);
}

void Wheel::Engine::Subsystems::Constraint2DResolver::WarmStartJoint(
    std::vector<Wheel::Engine::Physics::JointTempCalculations>& jointCalcs, size_t jcIdx, int j)
{
    Physics::JointTempCalculations& jointCalc = jointCalcs[j];
    auto cached = m_JointCache.find(jcIdx);
    if (cached != m_JointCache.end())
    {
        Physics::JointConstraintSolver::WarmStart(jointCalc, cached->second.impulse);
    }
}

void Wheel::Engine::Subsystems::Constraint2DResolver::ResolveCollisionConstraints(float a_DeltaTime, std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs, int i)
{
    size_t tcIdx = 0;
    for (int j = 0; j < m_Manifolds->size(); j++)
    {
        Collision::Collision2DManifold collision2DManifold = (*m_Manifolds)[j];
        if (!collision2DManifold.isColliding)
            continue;
        if (i == 0)
        {
            tempCalcs.push_back(Physics::CollisionConstraintSolver::PrepareConstraintSolver((*m_Manifolds)[j],
                m_TransformPool->GetComponent((*m_Manifolds)[j].collider1), m_TransformPool->GetComponent((*m_Manifolds)[j].collider2),
                m_RigidbodyPool->GetComponent((*m_Manifolds)[j].collider1), m_RigidbodyPool->GetComponent((*m_Manifolds)[j].collider2),
                a_DeltaTime));
            WarmStartCollision(tempCalcs, tcIdx, j);
        }

        Physics::CollisionConstraintSolver::SolvePseudoVelocities(tempCalcs[tcIdx]);

        auto& manifold = (*m_Manifolds)[j];
        if (manifold.contactCount == 1)
        {
            Engine::Physics::CollisionConstraintSolver::Solve1ContactConstraint(tempCalcs[tcIdx]);
        }
        else if (manifold.contactCount == 2)
        {
            Engine::Physics::CollisionConstraintSolver::Solve2ContactConstraint(tempCalcs[tcIdx]);
        }
        if (i + 1 == MAX_CONSTRAINT_ITERATION)
        {
            if (collision2DManifold.contactCount == 1) Physics::CollisionConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],0);
            else
            {
                Physics::CollisionConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],0);
                Physics::CollisionConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],1);
            }
        }
        tcIdx++;

    }
}

void Wheel::Engine::Subsystems::Constraint2DResolver::ResolveFrictionConstraints(std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs)
{
    for (auto& tc : tempCalcs)
    {
        int contactCount = tc.Manifold.contactCount;
        for (int i = 0; i < contactCount; i++)
            Physics::CollisionConstraintSolver::SolveFrictionConstraint(tc, i);
    }
}

void Wheel::Engine::Subsystems::Constraint2DResolver::CacheContactImpulses(std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs)
{
    // Persist this frame's impulses for next-frame warm starting.
    std::unordered_map<uint64_t, Physics::CachedContact> nextCache;
    nextCache.reserve(tempCalcs.size());
    for (auto& tc : tempCalcs)
    {
        Collision::Collision2DManifold& manifold = tc.Manifold;
        Physics::CachedContact cached;
        for (int i = 0; i < manifold.contactCount; i++)
        {
            cached.id[i]              = manifold.contactId[i];
            cached.normalImpulse[i]   = tc.impulses[i];
            cached.frictionImpulse[i] = tc.frictionImpulses[i];
        }
        nextCache[Wheel::Engine::Physics::MakeContactKey(manifold.collider1, manifold.collider2)] = cached;
    }
    m_ContactCache.swap(nextCache);
}

void Wheel::Engine::Subsystems::Constraint2DResolver::CacheJointImpulses(std::vector<Wheel::Engine::Physics::JointTempCalculations>& jointCalcs)
{
    // Persist this frame's accumulated joint impulse, keyed by owner entity, for
    // next-frame warm starting. Rebuilt each frame so removed joints drop out.
    std::unordered_map<uint32_t, Physics::CachedJoint> nextCache;
    nextCache.reserve(jointCalcs.size());
    for (auto& jc : jointCalcs)
    {
        if (jc.entity == NO_VALUE)
            continue;
        Physics::CachedJoint cached;
        cached.impulse = jc.cachedImpulse;
        nextCache[jc.entity] = cached;
    }
    m_JointCache.swap(nextCache);
}

void Wheel::Engine::Subsystems::Constraint2DResolver::ResolveDistanceJointConstraints(float a_DeltaTime,
    std::vector<Wheel::Engine::Physics::JointTempCalculations>& jointCalcs, int i)
{
    if (!m_JointPool)
        return;
    int j = 0;
    for (auto entity : m_JointEntities)
    {
        if (i == 0)
        {
            Components::DistanceJoint2D& joint = m_JointPool->GetComponent(entity);
            Components::Transform2D& transform1 = m_TransformPool->GetComponent(entity);
            Components::Rigidbody2D& rigidbody1 = m_RigidbodyPool->GetComponent(entity);
            Components::Transform2D& transform2 = joint.connectedRigidbody == NO_VALUE ? transform1 : m_TransformPool->GetComponent(joint.connectedRigidbody);
            Components::Rigidbody2D& rigidbody2 = joint.connectedRigidbody == NO_VALUE ? rigidbody1 : m_RigidbodyPool->GetComponent(joint.connectedRigidbody);
            jointCalcs.push_back(Physics::JointConstraintSolver::PrepareJointConstraintSolver(transform1, transform2, rigidbody1, rigidbody2, joint, a_DeltaTime));
            jointCalcs.back().entity = entity;   // cache key for warm starting next frame
            WarmStartJoint(jointCalcs, entity, j);
        }
        Physics::JointConstraintSolver::SolveDistanceJointConstraint(jointCalcs[j]);
        j++;
    }
}

void Wheel::Engine::Subsystems::Constraint2DResolver::SolveConstraints(std::vector<Collision::Collision2DManifold>* a_Manifolds, Engine::Scene* a_Scene, float a_DeltaTime)
{
    m_Manifolds = a_Manifolds;
    bool checkCollision = true;
    if (m_Manifolds == nullptr || m_Manifolds->empty())
        checkCollision = false;
    if (!m_TransformPool)
    {
        m_TransformPool = a_Scene->GetComponentPool<Components::Transform2D>();
        m_RigidbodyPool = a_Scene->GetComponentPool<Components::Rigidbody2D>();
        m_JointPool = a_Scene->GetComponentPool<Components::DistanceJoint2D>();
    }
    std::vector<Physics::CollisionTempCalculations> tempCalcs = {};
    std::vector<Physics::JointTempCalculations> jointCalcs = {};
    for (int i = 0; i < MAX_CONSTRAINT_ITERATION; i++)
    {
        if (checkCollision) ResolveCollisionConstraints(a_DeltaTime, tempCalcs, i);
        ResolveDistanceJointConstraints(a_DeltaTime, jointCalcs, i);
    }

    //ResolveFrictionConstraints(tempCalcs);

    CacheContactImpulses(tempCalcs);
    CacheJointImpulses(jointCalcs);
}
