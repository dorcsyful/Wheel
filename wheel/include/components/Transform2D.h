#pragma once
#include <string>

#include "../math/Vector2.h"

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

            std::string name;
            Wheel::Math::Vector2 position;
            Wheel::Math::Vector2 scale;
            float rotation = 0.0f;

        private:
            template <typename T> friend class Engine::ComponentPool;

            void SetEntityId(uint32_t id) { m_Id = id; name = "Entity " + std::to_string(id); }

            uint32_t m_Id = 0;

        };
};

}
