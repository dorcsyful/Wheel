#pragma once
#include "DebugDescriptor.h"
#include "math/Vector4.h"

namespace Wheel
{
    namespace Components
    {
        enum class CameraMode : int
        {
            MODE_ORTHO,
            MODE_PERSPECTIVE
        };
        /**
         * @brief You can have multiple cameras in the scene, but only one can be active at a time. If multiple cameras are set to active, a random one will be chosen
         */
        class CameraComponent
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
            CameraMode mode = CameraMode::MODE_ORTHO;
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
            /**
             * @brief World units visible vertically. Resolution-independent
             */
            float orthoSize = 7.2f;
            //TODO: Implement required variables for 3D camera

                REFLECT_BEGIN(CameraComponent)
                FIELD(backgroundColor)
                FIELD(mode)
                FIELD(width)
                FIELD(height)
                FIELD(zoom)
                FIELD(orthoSize)
                REFLECT_END(CameraComponent, "CameraComponent")

        private:
            bool active = true;

        };
    }
}
