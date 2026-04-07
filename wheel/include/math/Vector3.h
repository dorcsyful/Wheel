#pragma once

namespace Wheel
{
    namespace Math
    {
        class Vector3
        {
        public:
            Vector3() : x(0), y(0), z(0) {};
            Vector3(float a_X, float a_Y, float a_Z) : x(a_X), y(a_Y), z(a_Z) {}
            float x;
            float y;
            float z;
        };
    }
}