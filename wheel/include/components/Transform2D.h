#pragma once
#include "../math/Vector2.h"


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
            Transform2D() {m_Id = -1; };
            Transform2D(uint32_t a_Id) { m_Id = a_Id; };
            Transform2D(uint32_t a_Id, float x, float y)
            {m_Id = a_Id; position.x = x; position.y = y; };
            Transform2D(uint32_t a_Id, float x, float y, float a_Rotation)
            {m_Id = a_Id; position.x = x; position.y = y; rotation = a_Rotation; };

            bool operator==(const Transform2D& a_Other) const { return m_Id == a_Other.m_Id; };
            bool operator!=(const Transform2D& a_Other) const { return m_Id != a_Other.m_Id; };

            //Transform2D(const Transform2D& a_Transform2D) = delete;
            //Transform2D(Transform2D&& a_Other) = delete;
            //Transform2D& operator=(Transform2D&& a_Other) = delete;
            //Transform2D& operator=(const Transform2D& a_Transform2D) = delete;
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

            Wheel::Math::Vector2 position;
            Wheel::Math::Vector2 scale;
            float rotation = 0.0f;

        private:
            uint32_t m_Id;

        };
};

}
