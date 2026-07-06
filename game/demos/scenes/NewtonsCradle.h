#pragma once
#include "../Demo.h"

namespace Wheel
{
    namespace Game
    {
        class NewtonsCradle : public Demo
        {
        public:
            NewtonsCradle() = default;
            ~NewtonsCradle() override = default;

            void Initialize(Core::Scene* scene) override;
            void GetCircleMaterial(uint32_t a_Id) {m_CircleMaterial = a_Id; }
        private:
            uint32_t SpawnObject(float x, float y, float width, float height);
            uint32_t m_CircleMaterial;
        };
    }
}
