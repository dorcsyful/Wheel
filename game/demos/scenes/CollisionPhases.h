#pragma once
#include "../Demo.h"

namespace Wheel
{
    namespace Game
    {
        class CollisionPhases : public Demo
        {
        public:
            CollisionPhases() = default;
            ~CollisionPhases() override = default;

            void Initialize(Core::Scene* scene) override {}
        };
    }
}
