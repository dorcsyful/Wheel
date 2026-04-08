#pragma once
#include <cstdint>

#include "Description.h"
#include "ComponentPool.h"
namespace Wheel
{
    namespace Engine
    {
        class System
        {
        public:
            virtual ~System() = default;
            System() = delete;
            System(Description a_Description) : m_Description(std::move(a_Description)) {}


            virtual void GetComponentPool(IComponentPool* a_Pool) = 0;
            virtual void Update(float deltaTime) = 0;
        protected:
            Description m_Description;
        };
    }
}