#pragma once
#include "core/System.h"
#include "core/ComponentPool.h"
#include "components/Collider2D.h"
#include "components/Transform2D.h"
#include "helpers/Collision2DManifold.h"

namespace Wheel
{
    namespace Engine
    {
       namespace Systems
        {
            class Collision2DSystem : public System
            {
            public:
                //Collision system needs access to Description to know the type of collider.
                //The per-entity description array is fetched lazily from the scene (see CheckNarrowPhase).
                explicit Collision2DSystem(const Description& description) : System(description) {}
                ~Collision2DSystem() override = default;

                void Update(float deltaTime) override;
                void SetDirtyFlag(Wheel::Engine::Description boxComponentDesc,
                                  Wheel::Engine::Description circleComponentDesc,
                                  bool enable);
                std::vector<Collision::Collision2DManifold>* GetManifolds() { return &m_Manifolds; }

            private:
                void CheckNarrowPhase();
                const std::array<Description, MAX_ENTITIES>* m_DescriptionRef = nullptr;
                std::vector<Collision::Collision2DManifold> m_Manifolds;
                ComponentPool<Components::BoxCollider2D>* m_BoxColliderPool  = nullptr;
                ComponentPool<Components::CircleCollider2D>* m_CircleColliderPool  = nullptr;
                ComponentPool<Components::Transform2D>*   m_TransformPool = nullptr;
            };
        }
    }

}
