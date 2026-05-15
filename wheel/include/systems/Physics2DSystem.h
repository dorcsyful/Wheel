#pragma once
#include "core/System.h"
#include "math/Vector2.h"

namespace Wheel::EventSystem
{
    struct SubscriptionToken;
}

namespace Wheel::Components
{
    struct Transform2D;
    struct Rigidbody2D;
    struct BoxCollider2D;
}
namespace Wheel
{
    namespace Engine
    {
        namespace Systems
        {
            class Physics2DSystem : public System
            {
            public:
                Physics2DSystem(const Engine::Description& a_Description) : Engine::System(a_Description)
                {
                }
                ~Physics2DSystem() override = default;

                void Update(float deltaTime) override;
            private:
                void ApplyGravity(Components::Rigidbody2D& a_Rigidbody2D,float deltaTime);
                void IntegrateVelocity(Components::Rigidbody2D& a_Rigidbody2D,float deltaTime);
                void IntegratePosition(Components::Transform2D& a_Transform2D, Components::Rigidbody2D& a_Rigidbody2D,float deltaTime);
                Math::Vector2 m_Gravity{0.0f, -9.81f};
                ComponentPool<Wheel::Components::Transform2D>* m_TransformPool = nullptr;
                ComponentPool<Wheel::Components::BoxCollider2D>* m_ColliderPool = nullptr;
                ComponentPool<Wheel::Components::Rigidbody2D>* m_RigidbodyPool = nullptr;

            };
        }
    }
}

