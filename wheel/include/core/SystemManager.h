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

            void AddEntityWithComponent(uint32_t a_EntityID, Description& a_Description)
            {
                for (int i = 0; i < m_SystemCount; i++)
                {
                    m_Systems[i]->AddEntityWithComponent(a_EntityID, a_Description);
                }
            }
            void RemoveEntityWithComponent(uint32_t a_EntityID, Description& a_Description)
            {
                for (int i = 0; i < m_SystemCount; i++)
                {
                    m_Systems[i]->RemoveEntityWithComponent(a_EntityID, a_Description);
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
            template <typename T>
            T* RegisterSystem(const Description& a_Description)
            {
                if (m_SystemCount < MAX_SYSTEMS)
                {
                    T* system = new T(a_Description);
                    m_Systems[m_SystemCount] = system;
                    m_SystemNames[m_SystemCount] = typeid(T).name();
                    m_SystemCount++;
                    return system;
                }
                return nullptr;
            }

            template<typename T>
            T* GetSystem()
            {
                const char* t_name = typeid(T).name();
                for (int i = 0; i < m_SystemCount; i++)
                {
                    if (t_name == m_SystemNames[i])
                    {
                        return static_cast<T*>(m_Systems[i]);
                    }
                }
            }

        private:
            System* m_Systems[MAX_SYSTEMS];
            const char* m_SystemNames[MAX_SYSTEMS];
            int m_SystemCount;
        };
    }
}


