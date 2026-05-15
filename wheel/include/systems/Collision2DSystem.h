#pragma once
#include "core/System.h"
#include "core/ComponentPool.h"
#include "components/Collider2D.h"
#include "components/Transform2D.h"

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

            private:
                void CheckNarrowPhase();

                ComponentPool<Components::BoxCollider2D>* m_ColliderPool  = nullptr;
                ComponentPool<Components::Transform2D>*   m_TransformPool = nullptr;
            };
        }
    }

}
