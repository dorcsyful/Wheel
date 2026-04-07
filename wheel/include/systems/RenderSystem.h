#pragma once
#include "core/System.h"
#include "../components/Transform2D.h"

namespace Wheel::Renderer
{
    class Renderer;
}

namespace Wheel::Components
{
    struct CameraComponent;
    struct Render2DComponent;
}

namespace Wheel
{
    namespace Engine
    {
        namespace Systems
        {
            /**
             * @brief The Engine communicates with the renderer through this system.
             */
            class RenderSystem : public Engine::System
            {
            public:
                RenderSystem() = delete;
                explicit RenderSystem(Description* a_Description);
                /**
                 * @return A pointer to the renderer. Use this to load resources.
                 */
                Renderer::Renderer* GetRenderer() { return m_Renderer; }
                void GetComponentPool(IComponentPool* a_Pool) override;
                void Update(float deltaTime) override;

            private:
                Renderer::Renderer* m_Renderer;
                ComponentPool<Components::Transform2D>* m_Transform2DPool;
                ComponentPool<Components::Render2DComponent>* m_RenderPool;
                ComponentPool<Components::CameraComponent>* m_CameraPool;
            };


        }
    }
}

