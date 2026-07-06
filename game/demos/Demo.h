#pragma once
#include "core/Scene.h"

namespace Wheel
{
    namespace Game
    {
        class Demo
        {
        public:
            Demo() = default;
            virtual ~Demo() = default;

            virtual void Initialize(Core::Scene* scene) = 0;
        protected:
            Core::Scene* m_Scene;
        };
    }
}
