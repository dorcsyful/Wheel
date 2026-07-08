#pragma once
#include <stdint.h>
#include "math/Vector2.h"
#include "debugger/DebugDescriptor.h"
#include "core/Globals.h"

namespace Wheel
{
    namespace Physics
    {
        /**
         * @brief Keeps the rigidbody at a fixed distance. Can be connected to a point or another rigidbody
         */
        struct DistanceJoint2D
        {
            //set this to NO_VALUE to connect it to a point
            uint32_t connectedRigidbody = NO_VALUE;
            //point on the connected body. If connectedRigidbody is NO_VALUE this is a point in world space
            Math::Vector2 otherAnchorPoint = Math::Vector2(0.0f, 0.0f);
            //point on the rigidbody relative to its center of mass
            Math::Vector2 localAnchorPoint = Math::Vector2(0.0f, 0.0f);
            float distance = 0.0f;
            //Whether to force max distance. Otherwise, the distance might become smaller than the set distance, but the joint will not allow it to become larger than the set distance. This is useful for rope-like behavior
            bool maxDistanceOnly = false;
            REFLECT_BEGIN(DistanceJoint2D)
            FIELD(connectedRigidbody)
            FIELD(otherAnchorPoint)
            FIELD(localAnchorPoint)
            FIELD(distance)
            FIELD(maxDistanceOnly)
            REFLECT_END(DistanceJoint2D, "DistanceJoint2D")
        };
    }
}
