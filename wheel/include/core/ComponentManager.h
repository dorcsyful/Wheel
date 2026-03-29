#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <typeinfo>
#include "ComponentPool.h"

namespace Wheel
{
    namespace Engine
    {
        class ComponentManager
        {
        public:
            ComponentManager()
            {
                m_ComponentPools = std::unordered_map<std::string, IComponentPool*>(0);
            }

            ~ComponentManager()
            {
                for (auto element : m_ComponentPools)
                {
                    delete element.second;
                }

                for (auto element : m_Descriptions)
                {
                    delete element.second;
                }
            }

            template <typename T>
            void RegisterComponent()
            {
                std::string name = typeid(T).name();
                assert(m_ComponentPools.find(name) == m_ComponentPools.end() && "Component type already registered.");

                ComponentPool<T>* componentPool = new ComponentPool<T>();
                 m_ComponentPools.insert(std::make_pair(name, componentPool));
                Description* description = new Description();
                std::bitset<MAX_COMPONENT_TYPES> componentBitset;
                componentBitset[m_ComponentPools.size() -1].flip();
                description->AddComponentType(componentBitset);
                m_Descriptions.insert(std::make_pair(name, description));
            }

            template <typename T>
            Description* GetDescription() const
            {
                return m_Descriptions.at(typeid(T).name());
            }

            template <typename T>
            T* AddComponent(uint32_t a_EntityId)
            {
                assert(a_EntityId < MAX_ENTITIES && "Entity ID out of range: ");
                std::string name = typeid(T).name();

                T* temp = static_cast<ComponentPool<T>*>(m_ComponentPools[name])->AddComponent(a_EntityId);
                return temp;
            }

            template <typename T>
            void RemoveComponent(T* a_Component)
            {
                assert(a_Component == nullptr && "Component cannot be null.");
                std::string name = typeid(T).name();
                static_cast<T*>(m_ComponentPools[name])->RemoveComponent(a_Component);
            }

            template <typename T>
            ComponentPool<T>* GetComponentPool()
            {
                auto type = std::string(typeid(T).name());
                assert(m_ComponentPools.find(type) != m_ComponentPools.end() && "Component type not found.");
                ComponentPool<T>* componentPool = static_cast<ComponentPool<T>*>(m_ComponentPools[type]);
                return componentPool;
            }

            template <typename T>
            T* GetComponent(uint32_t a_EntityId)
            {
                //TODO: UPDATE TO MATCH DESCRIPTION
                assert(a_EntityId < MAX_ENTITIES && "Entity ID out of range.");
                std::string name = typeid(T).name();
                return static_cast<ComponentPool<T>*>(m_ComponentPools[name])->GetComponent(a_EntityId);

            }

            void EntityDestroyed(uint32_t a_EntityId)
            {

                for (auto current : m_ComponentPools)
                {
                    current.second->EntityDestroyed(a_EntityId);
                }
            }

            uint8_t GetNumberOfComponentTypes()
            {
                return m_ComponentPools.size();
            }


        private:
            std::unordered_map<std::string, IComponentPool*> m_ComponentPools;
            std::unordered_map<std::string, Description*> m_Descriptions;
        };
    }
}

