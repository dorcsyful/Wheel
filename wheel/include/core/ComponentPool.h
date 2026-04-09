#pragma once
#include <cassert>
#include <cstdint>
#include <unordered_map>
#include "Globals.h"

namespace Wheel
{
    namespace Engine
    {
        //Needs a parent class so we can store it in ComponentManager
        class IComponentPool
        {
        public:
            IComponentPool() = default;
            virtual ~IComponentPool() {};
            virtual void EntityDestroyed(uint32_t a_Entity) = 0;
        };

        template <typename T>
        class ComponentPool : public IComponentPool
        {
        public:

            ComponentPool() : m_Components{NULL}
            {
                //m_Components = std::array<T, MAX_ENTITIES>();
                m_EntityToComponent = std::unordered_map<uint32_t, uint32_t>();
                m_ComponentToEntity = std::unordered_map<uint32_t, uint32_t>();
            }

            ~ComponentPool() override
            {

            }

            size_t Size() { return m_Size; }

            T& AddComponent(uint32_t a_Entity)
            {
                assert(m_EntityToComponent.find(a_Entity) == m_EntityToComponent.end() && "Entity already has component.");
                T component = T();
                m_EntityToComponent.insert(std::make_pair(a_Entity, m_Size));
                m_ComponentToEntity.insert(std::make_pair(m_Size, a_Entity));
                m_Components[m_Size] = component;
                m_Size++;
                return m_Components[m_Size];
            }

            T& GetComponent(uint32_t a_Entity)
            {
                return m_Components[m_EntityToComponent[a_Entity]];
            }

            void RemoveComponent(uint32_t a_Entity)
            {
                assert(
                    m_EntityToComponent.find(a_Entity) != m_EntityToComponent.end() &&
                    "Entity does not have component.");

                //find the position of the key components and entities
                uint32_t indexToRemove = m_EntityToComponent[a_Entity];
                uint32_t lastIndex = m_Size - 1;
                uint32_t lastEntity = m_ComponentToEntity[lastIndex];
                //delete component
                //m_Components[indexToRemove] = nullptr;

                //moving the last component in the array into the now empty spot
                m_Components[indexToRemove] = m_Components[lastIndex];
                //m_Components[lastIndex] = nullptr;
                //moving the removed components index into the last spot. This is so all values in the map remains unique
                m_EntityToComponent[lastEntity] = indexToRemove;
                //moving the last entity id into the place of the removed index
                m_ComponentToEntity[indexToRemove] = lastEntity;

                m_EntityToComponent.erase(a_Entity);
                m_ComponentToEntity.erase(m_Size - 1);
                m_Size--;
            }

            void EntityDestroyed(uint32_t a_Entity) override
            {
                if (m_EntityToComponent.find(a_Entity) != m_EntityToComponent.end())
                {
                    RemoveComponent(a_Entity);
                }
            }

            std::unordered_map<uint32_t, T*> GetComponents()
            {
                std::unordered_map<uint32_t, T*> components = std::unordered_map<uint32_t, T*>(m_Size);
                for (const auto & [ key, value ] : m_EntityToComponent) {
                    std::pair<uint32_t, T*> component = std::make_pair(key, &m_Components[value]);
                    components.insert(component);
                }

                return components;
            }

        private:
            T m_Components[MAX_ENTITIES];

            std::unordered_map<uint32_t, uint32_t> m_EntityToComponent;
            std::unordered_map<uint32_t, uint32_t> m_ComponentToEntity;

            uint32_t m_Size = 0;
        };
    }
}
