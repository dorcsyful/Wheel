#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "core/ComponentPool.h"
#include "systems/helpers/CollisionConstraintSolver.h"

namespace Wheel::Engine::Collision
{
    struct Collision2DManifold;
}

namespace Wheel::Components
{
    struct Rigidbody2D;
    struct Transform2D;
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
}
namespace Wheel
{
    namespace Engine
    {
        namespace Subsystems
        {
            class Constraint2DResolver {
            public:
                void SolveConstraints(std::vector<Collision::Collision2DManifold>* a_Manifolds, Engine::Scene* a_Scene, float a_DeltaTime);
                void IntegratePseudoPosition(Components::Transform2D& a_Transform2D, Components::Rigidbody2D& a_Rigidbody2D, float deltaTime);
                void WarmStart(std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs, size_t tcIdx, int j);
                void ResolveCollisionConstraints(float a_DeltaTime,
                                                 std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs,
                                                 int i);
                void ResolveFrictionConstraints(std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs);
                void CacheContactImpulses(std::vector<Wheel::Engine::Physics::TempCalculations>& tempCalcs);

            private:
                // Survives across frames to warm start each contact
                std::unordered_map<uint64_t, Physics::CachedContact> m_ContactCache;
                std::vector<Collision::Collision2DManifold>* m_Manifolds = nullptr;
                ComponentPool<Wheel::Components::Transform2D>* m_TransformPool = nullptr;
                ComponentPool<Wheel::Components::Rigidbody2D>* m_RigidbodyPool = nullptr;

            };
        }
    }
}


