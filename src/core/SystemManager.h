#pragma once
#include <vector>

#include "ComponentPool.h"
#include "System.h"
#include "Description.h"
namespace Wheel
{
    namespace Engine
    {
        class SystemManager
        {
        public:
            SystemManager() = default;
            ~SystemManager() = default;

            template <typename T>
            void AddComponentType(ComponentPool<T>* a_Pool){ throw std::exception(); }
            void Update(float deltaTime) { throw std::exception(); }
            void RegisterSystem(System* a_System, Description* a_Description) {  }

        private:
            System* m_Systems[MAX_SYSTEMS];
        };
    }
}


