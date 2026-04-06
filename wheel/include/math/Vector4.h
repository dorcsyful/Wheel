#pragma once

namespace Wheel
{
    namespace Math
    {
        class Vector4
        {
        public:
            Vector4() : x(0), y(0), z(0), w(0) {};
            Vector4(float a_X, float a_Y, float a_Z, float a_W) : x(a_X), y(a_Y), z(a_Z), w(a_W) {}
            float x;
            float y;
            float z;
            float w;
        };
    }
}