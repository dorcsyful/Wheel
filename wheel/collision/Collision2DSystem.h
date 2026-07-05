#pragma once
#include "core/System.h"
#include "core/ComponentPool.h"
#include "Collider2D.h"
#include "../common/Transform2D.h"
#include "Collision2DManifold.h"

namespace Wheel
{
   namespace Collision
    {
        class Collision2DSystem : public Core::System
        {
        public:
            //Collision system needs access to Description to know the type of collider.
            //The per-entity description array is fetched lazily from the scene (see CheckNarrowPhase).
            explicit Collision2DSystem(const Core::Description& description) : System(description) {}
            ~Collision2DSystem() override = default;

            void Update(float deltaTime) override;
            void SetDirtyFlag(Core::Description boxComponentDesc,
                              Core::Description circleComponentDesc,
                              bool enable);
            std::vector<Collision2DManifold>* GetManifolds() { return &m_Manifolds; }

        private:
            void CheckNarrowPhase();
            const std::array<Core::Description, MAX_ENTITIES>* m_DescriptionRef = nullptr;
            std::vector<Collision2DManifold> m_Manifolds;
            Core::ComponentPool<BoxCollider2D>* m_BoxColliderPool  = nullptr;
            Core::ComponentPool<CircleCollider2D>* m_CircleColliderPool  = nullptr;
            Core::ComponentPool<Common::Transform2D>*   m_TransformPool = nullptr;
        };
    }
}
