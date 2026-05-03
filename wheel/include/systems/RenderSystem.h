#pragma once
#include "RenderedObject.h"
#include "core/System.h"
namespace Wheel::Renderer
{
    class Renderer;
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
                explicit RenderSystem(const Description& a_Description);
                /**
                 * @return A pointer to the renderer. Use this to load resources.
                 */
                void GetRenderer(Renderer::Renderer* a_Renderer) { m_Renderer = a_Renderer; }
                void Update(float deltaTime) override;
                void SetCameraEntity(uint32_t a_Id) { m_CameraEntity = a_Id; }
                static bool ROSorter(Renderer::RenderedObject& a_A, Renderer::RenderedObject& a_B);
            private:
                Renderer::Renderer* m_Renderer = nullptr;
                uint32_t m_CameraEntity = -1;
                std::vector<Renderer::RenderedObject> m_RenderObjects;
                float m_designWidth = 0.0f;
                float m_designHeight = 0.0f;
                float m_initialZoom = 0.0f;
            };


        }
    }
}

