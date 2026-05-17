#pragma once
#include <string>

#include "DebugDescriptor.h"
#include "../math/Vector2.h"
#include "math/Matrix2x2.h"

namespace Wheel::Engine { template <typename T> class ComponentPool; }

namespace Wheel
{
    namespace Components
    {
        /**
         * @brief Stores the position, rotation and scale of an entity. Note: this component is unique to each entity and therefore does not support copy operators. You also cannot inherit from it.
         */
        class Transform2D final
        {
        public:
            Transform2D() = default;
            uint32_t GetEntityId() const { return m_Id; }
            bool operator==(const Transform2D& a_Other) const { return m_Id == a_Other.m_Id; };
            bool operator!=(const Transform2D& a_Other) const { return m_Id != a_Other.m_Id; };

            Transform2D operator+(const Transform2D& a_Other) const = delete;
            Transform2D operator-(const Transform2D& a_Other) const = delete;
            Transform2D operator*(const Transform2D& a_Other) const = delete;
            Transform2D operator/(const Transform2D& a_Other) const = delete;
            Transform2D operator%(const Transform2D& a_Other) const = delete;
            Transform2D operator+=(const Transform2D& a_Other) = delete;
            Transform2D operator-=(const Transform2D& a_Other) = delete;
            Transform2D operator*=(const Transform2D& a_Other) = delete;
            Transform2D operator/=(const Transform2D& a_Other) = delete;
            Transform2D operator%=(const Transform2D& a_Other) = delete;
            Transform2D operator++() = delete;
            Transform2D operator++(int) = delete;
            Transform2D operator--() = delete;
            Transform2D operator--(int) = delete;
            Transform2D operator<(const Transform2D& a_Other) = delete;
            Transform2D operator<=(const Transform2D& a_Other) = delete;
            Transform2D operator>=(const Transform2D& a_Other) = delete;
            Transform2D operator~() = delete;
            Transform2D operator^(const Transform2D& a_Other) = delete;
            Transform2D operator&(const Transform2D& a_Other) = delete;
            Transform2D operator|(const Transform2D& a_Other) = delete;
            Transform2D operator^=(const Transform2D& a_Other) = delete;
            Transform2D operator&=(const Transform2D& a_Other) = delete;
            Transform2D operator<<(const Transform2D& a_Other) = delete;
            Transform2D operator>>(const Transform2D& a_Other) = delete;
            Transform2D operator<<=(const Transform2D& a_Other) = delete;
            Transform2D operator>>=(const Transform2D& a_Other) = delete;

            // Field names match private member names so reflection displays them without a prefix
            REFLECT_BEGIN(Transform2D)
            FIELD(name)
            FIELD(position)
            FIELD(scale)
            FIELD(rotation)
            REFLECT_END(Transform2D, "Transform2D")

            std::string name;
            bool isDirty = true;

            const Math::Vector2& GetPosition() const { return position; }
            const Math::Vector2& GetScale() const { return scale; }
            float GetRotation() const { return rotation; }

            void SetPosition(const Wheel::Math::Vector2& pos) { position = pos; isDirty = true; }
            void SetPosition(float x, float y) { position.x = x; position.y = y; isDirty = true; }
            void SetScale(const Wheel::Math::Vector2& s){ scale = s; isDirty = true; }
            void SetScale(float x, float y) { scale.x = x; scale.y = y; isDirty = true; }
            void SetRotation(float rot)
            {
                rotation = rot; isDirty = true;
                m_RotationMatrix.CreateRotation(rotation);
            }
            Math::Matrix2x2 GetRotationMatrix() const { return m_RotationMatrix; }

        private:
            //Hate it but required so the user cannot change EntityId
            template <typename T> friend class Engine::ComponentPool;

            void SetEntityId(uint32_t id) { m_Id = id; name = "Entity " + std::to_string(id); }
            Math::Matrix2x2 m_RotationMatrix;
            uint32_t m_Id = UINT32_MAX;
            Wheel::Math::Vector2 position;
            Wheel::Math::Vector2 scale;
            float rotation = 0.0f;
        };
    };

}