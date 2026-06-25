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
void Wheel::Engine::Subsystems::Constraint2DResolver::IntegratePseudoPosition(Components::Transform2D& a_Transform2D,
    Components::Rigidbody2D& a_Rigidbody2D, float deltaTime)
{
    a_Transform2D.SetPosition(a_Transform2D.GetPosition() + a_Rigidbody2D.pseudoLinearVelocity * deltaTime);
    a_Transform2D.SetRotationInRadians(a_Transform2D.GetRotationInRadians() + a_Rigidbody2D.pseudoAngularVelocity * deltaTime);
    a_Rigidbody2D.pseudoLinearVelocity = Math::Vector2(0.0f, 0.0f);
    a_Rigidbody2D.pseudoAngularVelocity = 0.0f;

}

void Wheel::Engine::Subsystems::Constraint2DResolver::WarmStart(std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs, size_t tcIdx, int j)
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

void Wheel::Engine::Subsystems::Constraint2DResolver::ResolveCollisionConstraints(float a_DeltaTime, std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs, int i)
{
    size_t tcIdx = 0;
    for (int j = 0; j < m_Manifolds->size(); j++)
    {
        if (!m_Manifolds->at(j).isColliding)
            continue;
        if (i == 0)
        {
            tempCalcs.push_back(Physics::CollisionConstraintSolver::PrepareConstraintSolver((*m_Manifolds)[j],
                m_TransformPool->GetComponent((*m_Manifolds)[j].collider1), m_TransformPool->GetComponent((*m_Manifolds)[j].collider2),
                m_RigidbodyPool->GetComponent((*m_Manifolds)[j].collider1), m_RigidbodyPool->GetComponent((*m_Manifolds)[j].collider2),
                a_DeltaTime));
            WarmStart(tempCalcs, tcIdx, j);
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
            if (m_Manifolds->at(j).contactCount == 1) Physics::CollisionConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],0);
            else
            {
                Physics::CollisionConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],0);
                Physics::CollisionConstraintSolver::SolveFrictionConstraint(tempCalcs[tcIdx],1);
            }
        }
        tcIdx++;

    }
}

void Wheel::Engine::Subsystems::Constraint2DResolver::ResolveFrictionConstraints(std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs)
{
    for (auto& tc : tempCalcs)
    {
        int contactCount = tc.Manifold.contactCount;
        for (int i = 0; i < contactCount; i++)
            Physics::CollisionConstraintSolver::SolveFrictionConstraint(tc, i);
    }
}

void Wheel::Engine::Subsystems::Constraint2DResolver::CacheContactImpulses(std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs)
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

void Wheel::Engine::Subsystems::Constraint2DResolver::SolveConstraints(std::vector<Collision::Collision2DManifold>* a_Manifolds, Engine::Scene* a_Scene, float a_DeltaTime)
{
    m_Manifolds = a_Manifolds;
    if (m_Manifolds == nullptr || m_Manifolds->empty())
        return;
    if (!m_TransformPool)
    {
        m_TransformPool = a_Scene->GetComponentPool<Components::Transform2D>();
        m_RigidbodyPool = a_Scene->GetComponentPool<Components::Rigidbody2D>();
    }
    std::vector<Physics::TempCalculations> tempCalcs = {};

    for (int i = 0; i < MAX_CONSTRAINT_ITERATION; i++)
    {
        ResolveCollisionConstraints(a_DeltaTime, tempCalcs, i);
    }

    ResolveFrictionConstraints(tempCalcs);

    CacheContactImpulses(tempCalcs);
}
