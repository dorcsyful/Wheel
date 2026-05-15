#pragma once
#include "core/System.h"

namespace Wheel::EventSystem
{
    struct SubscriptionToken;
}

namespace Wheel
{
    namespace Engine
    {
        namespace Systems
        {
            class PhysicsSystem : public System
            {
                PhysicsSystem(const Engine::Description& a_Description) : Engine::System(a_Description)
                {
                }
                ~PhysicsSystem() override = default;

            };
        }
    }
}

