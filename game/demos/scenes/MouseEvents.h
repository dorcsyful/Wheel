#pragma once
#include "../Demo.h"
#include "core/Scene.h"

namespace Wheel
{
    namespace Game
    {
        class MouseEvents : public Demo
        {
            public:
            MouseEvents() = default;
            ~MouseEvents() override = default;

            void Initialize(Core::Scene* scene) override;
            void GetBaseMaterial(uint32_t baseMaterial);

        private:
            uint32_t m_BaseMaterial;
            std::vector<Wheel::EventSystem::SubscriptionToken> m_SubscriptionTokens;

        };
    }
}