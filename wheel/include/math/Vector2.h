#pragma once

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

            float operator[](int i) const { return i == 0 ? x : y; }
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

            float Dot(const Vector2& a) const { return x * a.x + y * a.y; }

            float x;
            float y;
        };
    }
}