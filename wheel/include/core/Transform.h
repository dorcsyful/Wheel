#pragma once
#include "../math/Vector2.h"


namespace Wheel
{
    namespace Engine
    {
        /**
         * @brief Stores the position, rotation and scale of an entity. Note: this component is unique to each entity and therefore does not support copy operators. You also cannot inherit from it.
         */
        class Transform final
        {
        public:
            Transform() = delete;
            Transform(uint32_t a_Id);
            Transform(uint32_t a_Id, float x, float y);
            Transform(uint32_t a_Id, float x, float y, float rotation);

            bool operator==(const Transform& a_Other) const { return m_Id == a_Other.m_Id; };
            bool operator!=(const Transform& a_Other) const { return m_Id != a_Other.m_Id; };

            Transform(const Transform& a_Transform) = delete;
            Transform(Transform&& a_Other) = delete;
            Transform& operator=(Transform&& a_Other) = delete;
            Transform& operator=(const Transform& a_Transform) = delete;
            Transform operator+(const Transform& a_Other) const = delete;
            Transform operator-(const Transform& a_Other) const = delete;
            Transform operator*(const Transform& a_Other) const = delete;
            Transform operator/(const Transform& a_Other) const = delete;
            Transform operator%(const Transform& a_Other) const = delete;
            Transform operator+=(const Transform& a_Other) = delete;
            Transform operator-=(const Transform& a_Other) = delete;
            Transform operator*=(const Transform& a_Other) = delete;
            Transform operator/=(const Transform& a_Other) = delete;
            Transform operator%=(const Transform& a_Other) = delete;
            Transform operator++() = delete;
            Transform operator++(int) = delete;
            Transform operator--() = delete;
            Transform operator--(int) = delete;
            Transform operator<(const Transform& a_Other) = delete;
            Transform operator<=(const Transform& a_Other) = delete;
            Transform operator>=(const Transform& a_Other) = delete;
            Transform operator~() = delete;
            Transform operator^(const Transform& a_Other) = delete;
            Transform operator&(const Transform& a_Other) = delete;
            Transform operator|(const Transform& a_Other) = delete;
            Transform operator^=(const Transform& a_Other) = delete;
            Transform operator&=(const Transform& a_Other) = delete;
            Transform operator<<(const Transform& a_Other) = delete;
            Transform operator>>(const Transform& a_Other) = delete;
            Transform operator<<=(const Transform& a_Other) = delete;
            Transform operator>>=(const Transform& a_Other) = delete;

            Wheel::Math::Vector2 position;
            Wheel::Math::Vector2 scale;
            float rotation;

        private:
            uint32_t m_Id;

        };
};

}
