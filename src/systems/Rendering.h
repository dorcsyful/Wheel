#pragma once
#include "../core/System.h"

namespace Wheel
{
    namespace Systems
    {
        class Rendering : public Engine::System
        {
            public:
            explicit Rendering(Description* a_Description) : System(a_Description)
            {
            }

            virtual ~Rendering() override { delete m_Description; }

            void GetComponentPool(Engine::IComponentPool* a_Pool) override;
            void Update(float deltaTime) override;

        private:
            Engine::ComponentPool<Engine::Transform> m_TransformPool;
        };
    }
}
