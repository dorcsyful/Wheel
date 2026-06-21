#pragma once
#include "DebugDescriptor.h"
#include "core/Globals.h"

namespace Wheel
{
    namespace Components
    {
        /**
         * @brief Keeps the rigidbody at a fixed distance. Can be connected to a point or another rigidbody
         */
        struct DistanceJoint2D
        {
            //set this to NO_VALUE to connect it to a point
            uint32_t connectedRigidbody = NO_VALUE;
            Math::Vector2 anchorPoint = Math::Vector2(0.0f, 0.0f);
            float distance = 0.0f;
            //Whether to force max distance. Otherwise, the distance might become smaller than the set distance, but the joint will not allow it to become larger than the set distance. This is useful for rope-like behavior
            bool maxDistanceOnly = true;

            REFLECT_BEGIN(DistanceJoint2D)
            FIELD(connectedRigidbody)
            FIELD(anchorPoint)
            FIELD(distance)
            FIELD(maxDistanceOnly)
            REFLECT_END(DistanceJoint2D, "DistanceJoint2D")
        };
    }
}
