#pragma once
#include <vector>
#include <cstdint>
#include "EventBus.h"
#include "RenderedObject.h"
#include "core/System.h"
#include "core/ComponentPool.h"
#include "components/Transform2D.h"
#include "components/Render2DComponent.h"
#include "components/CameraComponent.h"
#include "math/Vector2.h"

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
                // No-op: render-list construction is driven once per frame via Render(),
                // not per fixed physics step through the SystemManager.
                void Update(float deltaTime) override {}
                /**
                 * @brief Snapshot the current transforms. Call once immediately before each
                 * fixed physics step so Render() can interpolate between the last two states.
                 */
                void SavePreviousTransforms();
                /**
                 * @brief Build the interpolated render list and hand it to the renderer.
                 * @param a_Alpha blend factor in [0,1]: accumulator / fixedDeltaTime.
                 */
                void Render(float a_Alpha);
                void SetCameraEntity(uint32_t a_Id) { m_CameraEntity = a_Id; }
                static bool ROSorter(Renderer::RenderedObject& a_A, Renderer::RenderedObject& a_B);
            private:
                struct TransformSnapshot
                {
                    Math::Vector2 position;
                    float         rotation = 0.0f;
                    Math::Vector2 scale;
                };
                void EnsurePools();

                // Indexed directly by entityId (IDs are dense and capped at
                // MAX_ENTITIES). Flat + contiguous: no hashing, no per-entity
                // node allocation. m_PrevValid marks which slots hold a real
                // snapshot so a never-snapshotted entity isn't yanked to (0,0).
                std::vector<TransformSnapshot> m_PrevTransforms;
                std::vector<uint8_t>           m_PrevValid;
                Renderer::Renderer* m_Renderer = nullptr;
                uint32_t m_CameraEntity = -1;
                std::vector<Renderer::RenderedObject> m_RenderObjects;
                float m_designWidth  = 0.0f;
                float m_designHeight = 0.0f;
                float m_initialZoom  = 0.0f;
                std::vector<EventSystem::SubscriptionToken> m_Tokens = {};

                ComponentPool<Components::Transform2D>*      m_TransformPool = nullptr;
                ComponentPool<Components::Render2DComponent>* m_RenderPool   = nullptr;
                ComponentPool<Components::CameraComponent>*  m_CameraPool    = nullptr;
            };


        }
    }
}

