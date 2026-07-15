#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>

#include "core/System.h"
#include "core/ComponentPool.h"
#include "core/Globals.h"
#include "math/Vector2.h"
#include "Constraint2DResolver.h"

namespace Wheel::Engine::Collision
{
    struct Collision2DManifold;
}



namespace Wheel::EventSystem
{
    struct SubscriptionToken;
}

namespace Wheel
{
    namespace Common { class Transform2D; }
    namespace Physics { struct Rigidbody2D; }
    namespace Collision
    {
        struct BoxCollider2D;
        struct CircleCollider2D;
    }
}
namespace Wheel
{
    namespace Physics
    {
        class Physics2DSystem : public Core::System
        {
        public:
            Physics2DSystem(const Core::Description& a_Description) : System(a_Description)
            {
                m_CollisionResolver = new Constraint2DResolver();
            }
            ~Physics2DSystem() override { delete m_CollisionResolver; }

            bool UpdateRigidbodyInertia(std::vector<unsigned>::value_type id,
                                        Common::Transform2D& transform,
                                        Rigidbody2D& rigidbody);
            void EnsurePools();
            void Update(float deltaTime) override;
        private:
            void ApplyGravity(Rigidbody2D& a_Rigidbody2D,float deltaTime);
            void IntegrateVelocity(Rigidbody2D& a_Rigidbody2D,float deltaTime);
            void IntegratePosition(Common::Transform2D& a_Transform2D, Rigidbody2D& a_Rigidbody2D,float deltaTime);
            float CalculateBoxInertia(const Collision::BoxCollider2D& a_Collider, const Common::Transform2D& a_Transform, float mass);
            float CalculateCircleInertia(const Collision::CircleCollider2D& a_Collider, const Common::Transform2D& a_Transform, float mass);

            Constraint2DResolver* m_CollisionResolver;
            Math::Vector2 m_Gravity{0.0f, -9.81f};
            Core::ComponentPool<Common::Transform2D>* m_TransformPool = nullptr;
            Core::ComponentPool<Collision::BoxCollider2D>* m_BoxColliderPool = nullptr;
            Core::ComponentPool<Collision::CircleCollider2D>* m_CircleColliderPool = nullptr;
            Core::ComponentPool<Rigidbody2D>* m_RigidbodyPool = nullptr;

            const std::array<Core::Description, MAX_ENTITIES>* m_DescriptionRef = nullptr;
            Core::Description m_BoxColliderDesc;
            Core::Description m_CircleColliderDesc;

        };
    }
}

