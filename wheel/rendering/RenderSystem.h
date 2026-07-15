#pragma once
#include <vector>
#include <cstdint>
#include <functional>
#include <algorithm>
#include "../events/EventBus.h"
#include "_gfx/include/RenderedObject.h"
#include "core/System.h"
#include "core/Scene.h"
#include "core/ComponentPool.h"
#include "../common/Transform2D.h"
#include "Sprite.h"
#include "CameraComponent.h"
#include "math/Vector2.h"

namespace Wheel::Renderer
{
    class Renderer;
}

namespace Wheel
{
    namespace Rendering
    {
        /**
         * @brief The Engine communicates with the renderer through this system.
         */
        class RenderSystem : public Core::System
        {
        public:
            RenderSystem() = delete;
            explicit RenderSystem(const Core::Description& a_Description);
            /**
             * @return A pointer to the renderer. Use this to load resources.
             */
            void GetRenderer(Renderer::Renderer* a_Renderer) { m_Renderer = a_Renderer; }
            // render-list construction is driven once per frame via Render(),
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

            /**
             * @brief Register a render feature: any feature component that's present
             * on an entity, creates extra RenderedObject(s) derived from the base sprite
             * (highlight, glow, shadow...). Register all features before the first rendered frame
             */
            template <typename T>
            void RegisterFeature(Helpers::RenderLevel a_Order,
                std::function<void(const Renderer::RenderedObject&, const T&,
                                   std::vector<Renderer::RenderedObject>&)> a_Out)
            {
                RenderFeature feature;
                feature.order = a_Order;
                feature.has   = [](Core::Scene* a_Scene, uint32_t a_Entity)
                {
                    return a_Scene->HasComponent<T>(a_Entity);
                };
                feature.result  = [emit = std::move(a_Out)](Core::Scene* a_Scene, uint32_t a_Entity,
                              const Renderer::RenderedObject& a_Base,
                              std::vector<Renderer::RenderedObject>& a_Out)
                {
                    emit(a_Base, a_Scene->GetComponent<T>(a_Entity), a_Out);
                };
                m_Features.push_back(std::move(feature));
            }

            /**
             * @brief Convert a world-space point to screen-space pixels using the currently active camera.
             */
            Math::Vector2 WorldToScreen(const Math::Vector2& a_World);
            /**
             * @brief Convert a screen-space pixel point to world-space using the currently active camera.
             */
            Math::Vector2 ScreenToWorld(const Math::Vector2& a_Screen);
            Math::Vector2 ScreenToWorld(float a_X, float a_Y) { return ScreenToWorld(Math::Vector2(a_X, a_Y)); }

            static Math::Vector2 WorldToScreen(const Math::Vector2& a_World,
                                               const Common::Transform2D& a_CameraTransform,
                                               const CameraComponent& a_CameraComponent);
            static Math::Vector2 ScreenToWorld(const Math::Vector2& a_Screen,
                                               const Common::Transform2D& a_CameraTransform,
                                               const CameraComponent& a_CameraComponent);
        private:
            struct TransformSnapshot
            {
                Math::Vector2 position;
                float         rotation = 0.0f;
                Math::Vector2 scale;
            };

            // Type-erased render feature.
            struct RenderFeature
            {
                Helpers::RenderLevel order = 0;
                //needs access to scene to check for required component
                std::function<bool(Core::Scene*, uint32_t)> has;
                std::function<void(Core::Scene*, uint32_t,
                                   const Renderer::RenderedObject&,
                                   std::vector<Renderer::RenderedObject>&)> result;
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
            std::vector<RenderFeature> m_Features;
            std::vector<EventSystem::SubscriptionToken> m_Tokens = {};
            std::vector<Renderer::RenderedObject> m_RenderFeatures;
            Core::ComponentPool<Common::Transform2D>*      m_TransformPool = nullptr;
            Core::ComponentPool<Sprite>* m_RenderPool   = nullptr;
            Core::ComponentPool<CameraComponent>*  m_CameraPool    = nullptr;
        };
    }
}

