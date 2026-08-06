#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "core/ComponentPool.h"
#include "CollisionConstraintSolver.h"
#include "../events/EventBus.h"
#include "../events/Events.h"
#include "../events/SubscriptionToken.h"
#include "JointConstraintSolver.h"

namespace Wheel::Collision { struct Collision2DManifold; }
namespace Wheel::Core { class Scene; }
namespace Wheel::Physics { struct CachedContact; }
namespace Wheel::Physics
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
    namespace Physics
    {
        class Constraint2DResolver {
        public:
            Constraint2DResolver();
            void SolveConstraints(std::vector<Collision::Collision2DManifold>* a_Manifolds, Core::Scene* a_Scene, float a_DeltaTime);
            void IntegratePseudoPosition(Common::Transform2D& a_Transform2D, Rigidbody2D& a_Rigidbody2D, float deltaTime);
            void WarmStartCollision(std::vector<CollisionTempCalculations>& tempCalcs, size_t tcIdx, int j);
            void WarmStartJoint(std::vector<JointTempCalculations>& jointCalcs, size_t jcIdx, int j);
            void ResolveCollisionConstraints(float a_DeltaTime,
                                             std::vector<CollisionTempCalculations>& tempCalcs,
                                             int i);
            void ResolveFrictionConstraints(std::vector<CollisionTempCalculations>& tempCalcs);
            void CacheContactImpulses(std::vector<CollisionTempCalculations>& tempCalcs);
            void CacheJointImpulses(std::vector<JointTempCalculations>& jointCalcs);
            void ResolveDistanceJointConstraints(float a_DeltaTime, std::vector<JointTempCalculations>& jointCalcs, int i);

        private:
            // Survives across frames to warm start each contact
            std::unordered_map<uint64_t, CachedContact> m_ContactCache;
            std::unordered_map<uint32_t, CachedJoint> m_JointCache;
            std::vector<Collision::Collision2DManifold>* m_Manifolds = nullptr;
            Core::ComponentPool<Common::Transform2D>* m_TransformPool = nullptr;
            Core::ComponentPool<Rigidbody2D>* m_RigidbodyPool = nullptr;
            Core::ComponentPool<DistanceJoint2D>* m_JointPool = nullptr;

            // Insertion-ordered (not unordered_set) so joint solve order is
            // deterministic and identical across stdlib implementations —
            // Gauss-Seidel solving is order-dependent when joints share a body.
            std::vector<uint32_t> m_JointEntities;
            EventSystem::SubscriptionToken m_Tokens;

        };
    }
}


