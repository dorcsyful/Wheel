#pragma once

namespace Wheel
{
    namespace Components
    {
        /**
         * @brief You can have multiple cameras in the scene, but only one can be active at a time. If multiple cameras are set to active, a random one will be chosen
         */
        struct CameraComponent
        {
            bool active;
        };
    }
}