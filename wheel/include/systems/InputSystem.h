#pragma once
#include "core/System.h"

namespace Wheel
{
    namespace Systems
    {
        //The input system relies on GLFW. Changing the renderer requires rewriting this class
        class InputSystem : Engine::System
        {
         public:
            InputSystem();
            ~InputSystem() override;

            void Update();

        private:
        };
    }
}
