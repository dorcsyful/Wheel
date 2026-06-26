#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "core/ComponentPool.h"
#include "systems/helpers/CollisionConstraintSolver.h"
#include "EventBus.h"
#include "Events.h"
#include "SubscriptionToken.h"
#include "systems/helpers/JointConstraintSolver.h"

namespace Wheel::Engine::Collision
{
    struct Collision2DManifold;
}

namespace Wheel::Components
{
    struct Rigidbody2D;
    struct Transform2D;
    struct DistanceJoint2D;
}

namespace Wheel::Engine
{
    class Scene;
}

namespace Wheel::Engine::Physics
{
    struct CachedContact;
}
namespace Wheel::Engine::Physics
{
    // Per-pair impulse memory for warm starting, keyed by contact feature ID so a
    // contact is matched to the same physical feature across frames.
    struct CachedContact
    {
        uint16_t id[2]              = { 0xFFFF, 0xFFFF };
        float    normalImpulse[2]   = { 0.0f, 0.0f };
        float    frictionImpulse[2] = { 0.0f, 0.0f };
    };
    struct CachedJoint
    {
        uint16_t id[2]              = { 0xFFFF, 0xFFFF };
        float impulse = 0.0f;
    };
}
namespace Wheel
{
    namespace Engine
    {
        namespace Subsystems
        {
            class Constraint2DResolver {
            public:
                Constraint2DResolver();
                void SolveConstraints(std::vector<Collision::Collision2DManifold>* a_Manifolds, Engine::Scene* a_Scene, float a_DeltaTime);
                void IntegratePseudoPosition(Components::Transform2D& a_Transform2D, Components::Rigidbody2D& a_Rigidbody2D, float deltaTime);
                void WarmStartCollision(std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs, size_t tcIdx, int j);
                void WarmStartJoint(std::vector<Wheel::Engine::Physics::JointTempCalculations>& jointCalcs, size_t jcIdx, int j);
                void ResolveCollisionConstraints(float a_DeltaTime,
                                                 std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs,
                                                 int i);
                void ResolveFrictionConstraints(std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs);
                void CacheContactImpulses(std::vector<Wheel::Engine::Physics::CollisionTempCalculations>& tempCalcs);
                void CacheJointImpulses(std::vector<Wheel::Engine::Physics::JointTempCalculations>& jointCalcs);
                void ResolveDistanceJointConstraints(float a_DeltaTime, std::vector<Wheel::Engine::Physics::JointTempCalculations>& jointCalcs, int i);

            private:
                // Survives across frames to warm start each contact
                std::unordered_map<uint64_t, Physics::CachedContact> m_ContactCache;
                std::unordered_map<uint32_t, Physics::CachedJoint> m_JointCache;
                std::vector<Collision::Collision2DManifold>* m_Manifolds = nullptr;
                ComponentPool<Wheel::Components::Transform2D>* m_TransformPool = nullptr;
                ComponentPool<Wheel::Components::Rigidbody2D>* m_RigidbodyPool = nullptr;
                ComponentPool<Wheel::Components::DistanceJoint2D>* m_JointPool = nullptr;

                std::unordered_set<uint32_t> m_JointEntities;
                EventSystem::SubscriptionToken m_Tokens;

            };
        }
    }
}


