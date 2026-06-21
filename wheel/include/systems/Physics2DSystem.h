#pragma once
#include <cstdint>
#include <unordered_map>

#include "core/System.h"
#include "math/Vector2.h"
#include "subsystems/Collision2DResolver.h"

namespace Wheel::Engine::Collision
{
    struct Collision2DManifold;
}



namespace Wheel::EventSystem
{
    struct SubscriptionToken;
}

namespace Wheel::Components
{
    struct Transform2D;
    struct Rigidbody2D;
    struct BoxCollider2D;
    struct CircleCollider2D;
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
                    m_CollisionResolver = new Subsystems::Collision2DResolver();
                }
                ~Physics2DSystem() override { delete m_CollisionResolver; }

                bool UpdateRigidbodyInertia(std::vector<unsigned>::value_type id,
                                            Wheel::Components::Transform2D& transform,
                                            Wheel::Components::Rigidbody2D& rigidbody);
                void EnsurePools();
                void Update(float deltaTime) override;
            private:
                void ApplyGravity(Components::Rigidbody2D& a_Rigidbody2D,float deltaTime);
                void IntegrateVelocity(Components::Rigidbody2D& a_Rigidbody2D,float deltaTime);
                void IntegratePosition(Components::Transform2D& a_Transform2D, Components::Rigidbody2D& a_Rigidbody2D,float deltaTime);
                float CalculateBoxInertia(const Components::BoxCollider2D& a_Collider, const Components::Transform2D& a_Transform, float mass);
                float CalculateCircleInertia(const Components::CircleCollider2D& a_Collider, const Components::Transform2D& a_Transform, float mass);

                Subsystems::Collision2DResolver* m_CollisionResolver;
                Math::Vector2 m_Gravity{0.0f, -9.81f};
                ComponentPool<Wheel::Components::Transform2D>* m_TransformPool = nullptr;
                ComponentPool<Wheel::Components::BoxCollider2D>* m_BoxColliderPool = nullptr;
                ComponentPool<Wheel::Components::CircleCollider2D>* m_CircleColliderPool = nullptr;
                ComponentPool<Wheel::Components::Rigidbody2D>* m_RigidbodyPool = nullptr;

            };
        }
    }
}

