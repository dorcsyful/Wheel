#pragma once
#include "debug/DebugDescriptor.h"
#include "math/Vector2.h"


namespace Wheel
{
    namespace Physics
    {
        enum class Rigidbody2DType : int { STATIC, DYNAMIC, KINEMATIC };

        struct Rigidbody2D
        {
        public:
            bool active = true;
            float friction = 0.5f;
            float restitution = 0.02f;
            float linearDamping = 0.02f;
            float angularDamping = 0.05f;
            Math::Vector2 centerOfMass = Math::Vector2(0.0f, 0.0f);
            bool affectedByGravity = true;
            Math::Vector2 linearVelocity = Math::Vector2(0.0f, 0.0f);
            float angularVelocity = 0.0f;
            Math::Vector2 pseudoLinearVelocity = Math::Vector2(0.0f, 0.0f);
            float pseudoAngularVelocity = 0.0f;
            bool isDirty = true;

            float GetInertia() const { return invInertia != 0.0f ? 1.0f / invInertia : 0.0f; }
            float GetMass() const { return invMass != 0.0f ? 1.0f / invMass : 0.0f; }
            float GetInverseMass() const { return invMass; }
            void SetMass(float a_Mass) { invMass = 1.0f / a_Mass; }
            void SetInertia(float a_Inertia) { invInertia = a_Inertia != 0.0f ? 1.0f / a_Inertia : 0.0f; }
            float GetInverseInertia() const { return invInertia; }
            void SetType(Rigidbody2DType a_Type)
            {
                rigidbodyType = a_Type;
                if (a_Type == Rigidbody2DType::STATIC)
                {
                    invMass = 0.0f;
                    invInertia = 0.0f;
                    force = Math::Vector2(0.0f, 0.0f);
                    torque = 0.0f;
                }
            }
            Rigidbody2DType GetType()
            {
                return rigidbodyType;
            }
            void AddForce(const Math::Vector2& a_Force) { force += a_Force; }
            void AddForceAtLocalPoint(const Math::Vector2& a_Force, const Math::Vector2& a_LocalPoint)
            {
                Math::Vector2 pressurePoint = a_LocalPoint - centerOfMass;
                force += a_Force;
                torque += pressurePoint.Cross(a_Force);

            }
            void AddTorque(float a_Torque) { torque += a_Torque; }
            void ClearForces() { force = Math::Vector2(0.0f, 0.0f); torque = 0.0f; }
            Math::Vector2 GetForce() const { return force; }
            float GetTorque() const { return torque; }

            REFLECT_BEGIN(Rigidbody2D)
                FIELD(active)
            FIELD(invMass)
            FIELD(friction)
            FIELD(restitution)
            FIELD(linearDamping)
            FIELD(angularDamping)
            FIELD(invInertia)
            FIELD(rigidbodyType)
            FIELD(centerOfMass)
            FIELD(linearVelocity)
            FIELD(angularVelocity)
            REFLECT_END(Rigidbody2D, "Rigidbody2D")
        private:
            Rigidbody2DType rigidbodyType = Rigidbody2DType::DYNAMIC;
            float invInertia = 1.0f;
            float invMass = 1.0f;
            Math::Vector2 force;
            float torque = 0.0f;
            Math::Vector2 impulse;

        };
    }
}