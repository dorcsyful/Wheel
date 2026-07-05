#pragma once
#include "debug/DebugDescriptor.h"
#include "math/Vector4.h"

namespace Wheel
{
    namespace Rendering
    {
        struct Highlight2D
        {
            public:
            Highlight2D(int a_Thickness, Math::Vector4 a_Color) : thickness(a_Thickness), color(a_Color) {}
            Highlight2D(int a_Thickness, float a_Red, float a_Green, float a_Blue, float a_Alpha)
                : thickness(a_Thickness), color(Math::Vector4(a_Red, a_Green, a_Blue, a_Alpha)){}
            Highlight2D() : thickness(0), color(Math::Vector4()){}
            bool active = true;
            int thickness = 0;
            Math::Vector4 color;
            REFLECT_BEGIN(Highlight2D)
            FIELD(thickness)
            FIELD(color)
            FIELD(active)
            REFLECT_END(Highlight2D, "Highlight2D")
        };
    }
}
