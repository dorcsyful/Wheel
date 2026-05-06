#pragma once
#include "core/System.h"

namespace Wheel
{
    namespace Engine
    {
       namespace Systems
        {
            class Collision2DSystem : public System
            {
            public:
                explicit Collision2DSystem(const Description& description) : System(description) {}
                ~Collision2DSystem() override = default;

                void Update(float deltaTime) override;

            private:

            };
        }
    }

}
