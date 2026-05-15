#pragma once
#include "DebugDescriptor.h"
#include "math/Vector2.h"


namespace Wheel
{
    namespace Components
    {
        enum class Rigidbody2DType : int { STATIC, DYNAMIC, KINEMATIC };

        struct Rigidbody2D
        {
        public:
            bool active = true;
            float mass = 1.0f;
            float friction = 0.5f;
            float restitution = 0.2f;
            float linearDamping = 0.02f;
            float angularDamping = 0.05f;
            float inertia = 1.0f;
            Rigidbody2DType rigidbodyType = Rigidbody2DType::DYNAMIC;
            Wheel::Math::Vector2 centerOfMass = Wheel::Math::Vector2(0.0f, 0.0f);

            Math::Vector2 linearVelocity = Math::Vector2(0.0f, 0.0f);
            float angularVelocity = 0.0f;


            REFLECT_BEGIN(Rigidbody2D)
            FIELD(active)
            FIELD(mass)
            FIELD(friction)
            FIELD(restitution)
            FIELD(linearDamping)
            FIELD(angularDamping)
            FIELD(inertia)
            FIELD(rigidbodyType)
            FIELD(centerOfMass)
            FIELD(linearVelocity)
            FIELD(angularVelocity)
            REFLECT_END(Rigidbody2D, "Rigidbody2D")

            void AddForce(const Math::Vector2& a_Force) { force += a_Force; }
            void AddTorque(float a_Torque) { torque += a_Torque; }
            void ClearForces() { force = Math::Vector2(0.0f, 0.0f); torque = 0.0f; }
            Math::Vector2 GetForce() const { return force; }
            float GetTorque() const { return torque; }
        private:
            Math::Vector2 force;
            float torque = 0.0f;
            Math::Vector2 impulse;

        };
    }
}