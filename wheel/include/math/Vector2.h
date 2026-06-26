#pragma once
#include <cmath>

namespace Wheel
{
    namespace Math
    {
        class Vector2
        {
        public:
            Vector2() : x(0), y(0) {}
            Vector2(float a_X, float a_Y) : x(a_X), y(a_Y) {}
            Vector2(const Vector2& other) : x(other.x), y(other.y) {}

            float& operator[](int i) { return i == 0 ? x : y; }
            const float& operator[](int i) const { return i == 0 ? x : y; }
            Vector2 operator-() const { return Vector2(-x, -y); }
            Vector2 operator/(float a) const { return Vector2(x / a, y / a); }
            Vector2 operator*(float a) const { return Vector2(x * a, y * a); }
            Vector2 operator+(const Vector2& a) const { return Vector2(x + a.x, y + a.y); }
            Vector2 operator-(const Vector2& a) const { return Vector2(x - a.x, y - a.y); }
            bool operator==(const Vector2& a) const { return x == a.x && y == a.y; }
            bool operator!=(const Vector2& a) const { return !(*this == a); }
            Vector2& operator=(const Vector2& a) { x = a.x; y = a.y; return *this; }
            Vector2& operator+=(const Vector2& a) { x += a.x; y += a.y; return *this; }
            Vector2& operator-=(const Vector2& a) { x -= a.x; y -= a.y; return *this; }
            Vector2& operator*=(float a) { x *= a; y *= a; return *this; }
            Vector2& operator/=(float a) { x /= a; y /= a; return *this; }
            Vector2 operator+(float a_Rhs) const { return Vector2(x + a_Rhs, y + a_Rhs); }
            Vector2 operator-(float a_Rhs) const { return Vector2(x - a_Rhs, y - a_Rhs); }

            float Dot(const Vector2& a) const { return x * a.x + y * a.y; }
            float Cross(const Vector2& a) const { return x * a.y - y * a.x; }
            float Length() const
            {
                float temp = x * x + y * y;
                return temp < 1e-6f ? 1e-6f : std::sqrt(temp);
            }

            float LengthSquared() const { return x * x + y * y; }
            Vector2 Normalized() const { return *this / Length(); }
            static Vector2 Normalize(const Vector2& a) { return a / a.Length(); }
            Vector2 Perpendicular() const { return Vector2(-y, x); }
            static Vector2 Lerp(const Vector2& a, const Vector2& b, float t) { return a + (b - a) * t; }

            float x;
            float y;
        };

        inline Wheel::Math::Vector2 operator*(float a_Lhs, const Wheel::Math::Vector2& a_Rhs)
        {
            return a_Rhs * a_Lhs;
        }
    }
}
