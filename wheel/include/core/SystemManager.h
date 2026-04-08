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
            SystemManager() : m_Systems{}, m_SystemCount(0)
            {
                for (int i = 0; i < MAX_SYSTEMS; i++)
                {
                    m_Systems[i] = nullptr;
                }
            }

            ~SystemManager()
            {
                for (int i = 0; i < m_SystemCount; i++)
                {
                    delete m_Systems[i];
                }
            }

            template <typename T>
            void AddComponentType(ComponentPool<T>* a_Pool)
            {
                // Store component pools for systems to use if needed
                // Currently a placeholder - implement when system-component interaction is needed
            }

            void Update(float deltaTime)
            {
                for (int i = 0; i < m_SystemCount; i++)
                {
                    m_Systems[i]->Update(deltaTime);

                }
            }

            void RegisterSystem(Wheel::Engine::System* a_System, Description a_Description)
            {
                if (m_SystemCount < MAX_SYSTEMS)
                {
                    m_Systems[m_SystemCount] = a_System;
                    m_SystemCount++;
                }
            }

        private:
            System* m_Systems[MAX_SYSTEMS];
            int m_SystemCount;
        };
    }
}


