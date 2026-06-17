#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <typeinfo>
#include "ComponentPool.h"
#include "Description.h"
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
                for (const auto& it : m_ComponentPools)
                {
                    delete it.second;
                }
            }

            template <typename T>
            void RegisterComponent()
            {
                std::string name = typeid(T).name();
                assert(m_ComponentPools.find(name) == m_ComponentPools.end() &&
                        "Component type already registered.");
                std::bitset<MAX_COMPONENT_TYPES> componentBitset;
                componentBitset[m_ComponentPools.size()].flip();

                Description description;
                description.AddComponentType(componentBitset);

                ComponentPool<T>* componentPool = new ComponentPool<T>(description);
                m_ComponentPools.insert(std::make_pair(name, componentPool));
                m_Descriptions.emplace(name, description);
            }

            template <typename T>
            [[nodiscard]] Description& GetDescription()
            {
                return m_Descriptions.at(typeid(T).name());
            }

            template <typename T>
            T& AddComponent(uint32_t a_EntityId)
            {
                assert(a_EntityId < MAX_ENTITIES && "Entity ID out of range: ");
                std::string name = typeid(T).name();

                T& temp = static_cast<ComponentPool<T>*>(m_ComponentPools[name])->AddComponent(a_EntityId);
                return temp;
            }

            template <typename T>
            void RemoveComponent(const T& a_Component)
            {
                assert(a_Component == nullptr && "Component cannot be null.");
                std::string name = typeid(T).name();
                static_cast<T>(m_ComponentPools[name])->RemoveComponent(a_Component);
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
            Description& GetComponentBitset()
            {
                return m_Descriptions.at(typeid(T).name());
            }

            template <typename T>
            T& GetComponent(uint32_t a_EntityId)
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

            std::string GetNameByDescription(const Description& description)
            {
                std::string name;
                for (const auto& pair : m_Descriptions)
                {
                    if (pair.second == description)
                    {
                        name = pair.first;
                        break;
                    }
                }
                return name;
            }

            Description GetDescriptionByName(const std::string& name)
            {
                return m_Descriptions.at(name);
            }

            IComponentPool* GetPoolByDescription(const Description& description)
            {
                std::string name = GetNameByDescription(description);
                return m_ComponentPools[name];
            }
            void* GetComponentRaw(uint32_t a_EntityId, const Description& a_Description)
            {
                IComponentPool* pool = GetPoolByDescription(a_Description);
                return pool->GetRaw(a_EntityId);
            }

        private:
            std::unordered_map<std::string, IComponentPool*> m_ComponentPools;
            std::unordered_map<std::string, Description> m_Descriptions;
        };
    }
}

