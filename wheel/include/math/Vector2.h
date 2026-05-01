#pragma once

namespace Wheel
{
    namespace Math
    {
        class Vector2
        {
        public:
            Vector2() : x(0), y(0) {};
            Vector2(float a_X, float a_Y) : x(a_X), y(a_Y) {}

            float operator[](int i) const { return i == 0 ? x : y; }
            Vector2 operator/(float a) const { return Vector2(x / a, y / a); }
            Vector2 operator*(float a) const { return Vector2(x * a, y * a); }
            Vector2 operator+(const Vector2& a) const { return Vector2(x + a.x, y + a.y); }
            Vector2 operator-(const Vector2& a) const { return Vector2(x - a.x, y - a.y); }

            float x;
            float y;
        };
    }
}