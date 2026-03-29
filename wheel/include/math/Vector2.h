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
            float x;
            float y;
        };
    }
}