#pragma once
#include <cassert>
#include <cstdint>
#include <unordered_map>
#include "Globals.h"
#include "Description.h"
#include <array>
namespace Wheel
{
    namespace Core
    {
        //Needs a parent class so we can store it in ComponentManager
        class IComponentPool
        {
        public:
            IComponentPool() = default;
            virtual ~IComponentPool() = default;
            virtual void EntityDestroyed(uint32_t a_Entity) = 0;
            virtual void* GetRaw(uint32_t entityId) = 0;

        };

        template <typename T>
        class ComponentPool : public IComponentPool
        {

            //SFINAE magic for getting the Entity id into Transform2D
            template <typename U, typename = void>
            struct HasSetEntityId : std::false_type {};
            template <typename U>
            struct HasSetEntityId<U, std::void_t<decltype(std::declval<U>().SetEntityId(uint32_t{}))>> : std::true_type {};

        public:
            ComponentPool(const Description& a_Description)
            {
                m_Description = a_Description;
                m_EntityToComponent = std::vector<uint32_t>(MAX_ENTITIES, NO_VALUE);
                m_ComponentToEntity = std::unordered_map<uint32_t, uint32_t>();
                m_Components.reserve(MAX_ENTITIES);
            }

            ~ComponentPool() override
            {

            }

            size_t Size() { return m_Size; }

            T& AddComponent(uint32_t a_Entity)
            {
                assert(m_EntityToComponent[a_Entity] == NO_VALUE && "Entity already has component.");
                m_EntityToComponent[a_Entity] = m_Size;
                m_ComponentToEntity.insert(std::make_pair(m_Size, a_Entity));
                m_Components.push_back(T{});
                if constexpr (HasSetEntityId<T>::value) {
                    m_Components[m_Size].SetEntityId(a_Entity);
                }
                m_Size++;
                return m_Components[m_Size - 1];
            }

            T& GetComponent(uint32_t a_Entity)
            {
                assert(m_EntityToComponent[a_Entity] != NO_VALUE && "Entity does not have component.");
                return m_Components[m_EntityToComponent[a_Entity]];
            }
            void* GetRaw(uint32_t entityId) override {
                return &GetComponent(entityId);
            }
            void RemoveComponent(uint32_t a_Entity)
            {
                assert(
                    m_EntityToComponent[a_Entity] != NO_VALUE && "Entity does not have component.");

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

                m_EntityToComponent[a_Entity] = NO_VALUE;
                m_ComponentToEntity.erase(m_Size - 1);
                m_Size--;
                m_Components.pop_back();
            }

            void EntityDestroyed(uint32_t a_Entity) override
            {
                if (m_EntityToComponent[a_Entity] != NO_VALUE)
                {
                    RemoveComponent(a_Entity);
                }
            }

            std::unordered_map<uint32_t, T*> GetComponents()
            {
                std::unordered_map<uint32_t, T*> components = std::unordered_map<uint32_t, T*>(m_Size);
                for (int i = 0; i < m_Size; i++){
                    std::pair<uint32_t, T*> component = std::make_pair(m_ComponentToEntity[i], &m_Components[i]);
                    components.insert(component);
                }

                return components;
            }
            Description GetDescription() const
            {
                return m_Description;
            }
        private:
            //T m_Components[MAX_ENTITIES];
            std::vector<T> m_Components;
            std::vector<uint32_t> m_EntityToComponent;
            std::unordered_map<uint32_t, uint32_t> m_ComponentToEntity;
            Description m_Description;
            uint32_t m_Size = 0;
        };
    }
}
