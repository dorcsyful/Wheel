#pragma once
#include "../Demo.h"
#include "debug/DebugDescriptor.h"

namespace Wheel
{
    namespace Game
    {
        class FallingColliders : public Demo
        {
        public:
            FallingColliders() = default;
            ~FallingColliders() override = default;

            void Initialize(Core::Scene* scene) override;
            void GetMaterials(uint32_t a_Base, uint32_t a_Circle)
            {
                m_BaseMat = a_Base; m_CircleMat = a_Circle;
            }

            void SpawnObject(const Math::Vector2& a_Position);

            REFLECT_BEGIN(FallingColliders)
            FIELD(spawnBox)
            FIELD(spawnedColliderHeight)
            FIELD(spawnedColliderWidth)
            FIELD(spawnedCircleRadius)
            REFLECT_END(FallingColliders, "FallingColliders")

        private:
            void CreateGroundCollider();
            void CreateWallColliders();
            void CreateCentralCircle();
            void CreateCircle(const Math::Vector2& a_Position);
            void CreateBox(const Math::Vector2& a_Position);
            bool spawnBox = true;
            float spawnedCircleRadius = 0.5f;
            float spawnedColliderWidth = 0.5;
            float spawnedColliderHeight = 0.7;

            uint32_t m_BaseMat;
            uint32_t m_CircleMat;;
            std::vector<EventSystem::SubscriptionToken> m_SubscriptionTokens {};
        };
    }
}
