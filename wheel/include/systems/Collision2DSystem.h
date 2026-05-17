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
                explicit Collision2DSystem(const Description& description) : System(description) {}
                ~Collision2DSystem() override = default;

                void Update(float deltaTime) override;
                std::vector<Collision::Collision2DManifold>* GetManifolds() { return &m_Manifolds; }

            private:
                void CheckNarrowPhase();
                std::vector<Collision::Collision2DManifold> m_Manifolds;
                ComponentPool<Components::BoxCollider2D>* m_ColliderPool  = nullptr;
                ComponentPool<Components::Transform2D>*   m_TransformPool = nullptr;
            };
        }
    }

}
