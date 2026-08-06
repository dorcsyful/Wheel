#pragma once
#include <cstdint>
#include <vector>

#include "math/Vector2.h"

namespace Wheel::Core { class Scene; }

namespace Wheel
{
    namespace QA
    {
        /**
         * @brief Base physics values for simulation without clutter from engine code
         */
        struct EntitySnapshot
        {
            uint32_t entityId = 0;
            Math::Vector2 position;
            float rotation = 0.0f;
            Math::Vector2 linearVelocity;
            float angularVelocity = 0.0f;
        };

        /**
         * @brief Builds a Scene with only the deterministic sim core registered:
         * Transform2D / BoxCollider2D / CircleCollider2D / Rigidbody2D / DistanceJoint2D
         * components, and Collision2DSystem + Physics2DSystem as the only systems —
         * Collision2DSystem is registered (and therefore updated)
         * before Physics2DSystem because Physics2DSystem::Update consumes this
         * frame's manifolds from it
         */
        Core::Scene* CreateHeadlessScene();

        /**
         * @brief Advances the scene numSteps times at a fixed timestep (never the
         * wall clock — the caller controls entirely how much simulated time
         * passes) and returns a snapshot of every entity with a Transform2D,
         * sorted by entity id. The sort makes the result independent of the
         * unordered containers used internally (ComponentPool::GetComponents),
         * so two calls with identical inputs are byte-identical regardless of
         * hash-bucket layout.
         */
        std::vector<EntitySnapshot> RunScene(Core::Scene& a_Scene, int a_NumSteps, float a_FixedDeltaTime = 1.0f / 60.0f);
    }
}