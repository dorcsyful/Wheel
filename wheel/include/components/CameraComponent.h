#pragma once
#include "math/Vector4.h"

namespace Wheel
{
    namespace Components
    {
        enum CameraMode
        {
            MODE_ORTHO,
            MODE_PERSPECTIVE
        };
        /**
         * @brief You can have multiple cameras in the scene, but only one can be active at a time. If multiple cameras are set to active, a random one will be chosen
         */
        struct CameraComponent
        {
        public:

            void SetCameraActive(bool a_Enable)
            {
                active = a_Enable;
            }
            bool IsActive() { return active; }

            Math::Vector4 backgroundColor = Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            /**
             * @brief Use Orthographic for 2D and Perspective for 3D
             */
            CameraMode mode = MODE_ORTHO;
            /**
             * @brief Vieport width
             */
            float width = 0;
            /**
             * @brief Viewport height
             */
            float height = 0;
            // float left = 0;
            // float right = 0;
            float zoom = 1.0f;
            //TODO: Implement required variables for 3D camera

        private:
            bool active = true;

        };
    }
}
