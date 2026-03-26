#pragma once
#include <cstdint>

#include "Description.h"
#include "Scene.h"

namespace Wheel
{
    namespace Engine
    {
        /**
         * @brief Abstract class. You must define the Update and GetComponentPool functions to instantiate it
         */
        class System
        {
        public:
            System() = delete;
            explicit System(Description* a_Description) : m_Description(a_Description) {}

            virtual ~System()
            {
                delete m_Description;
            }

            virtual void GetComponentPool(IComponentPool* a_Pool) = 0;
            virtual void Update(float deltaTime) = 0;
        protected:
            Description* m_Description;
        };
    }
}