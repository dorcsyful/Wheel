#pragma once
#include "DebugDescriptor.h"

namespace Wheel
{
    namespace Components
    {
        struct FixedJoint2D
        {
                uint32_t other;
                float restLength = 1.0f;
                float stiffness = 1.f;
                bool isDirty = true;

                REFLECT_BEGIN(FixedJoint2D)
                FIELD(other)
                FIELD(restLength)
                FIELD(stiffness)
                REFLECT_END(FixedJoint2D, "FixedJoint2D")
        };
    }
}
