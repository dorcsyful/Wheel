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
            bool active;
            float mass;
            float friction;
            float restitution;
            float linearDamping;
            float angularDamping;
            float inertia;
            Rigidbody2DType isKinematic;
            Wheel::Math::Vector2 centerOfMass;

            Math::Vector2 linearVelocity;
            float angularVelocity;


            REFLECT_BEGIN(Rigidbody2D)
            FIELD(active)
            FIELD(mass)
            FIELD(friction)
            FIELD(restitution)
            FIELD(linearDamping)
            FIELD(angularDamping)
            FIELD(inertia)
            FIELD(isKinematic)
            FIELD(centerOfMass)
            REFLECT_END(Rigidbody2D, "Rigidbody2D")

            void AddForce(const Math::Vector2& a_Force) { force += a_Force; }
            void AddTorque(float a_Torque) { torque += a_Torque; }

            Math::Vector2 GetForce() const { return force; }
            float GetTorque() const { return torque; }
        private:
            Math::Vector2 force;
            float torque = 0.0f;
            Math::Vector2 impulse;

        };
    }
}