#pragma once
#include <cstdint>
#include <iostream>
#include "SystemManager.h"
#include "ComponentManager.h"
#include "EntityManager.h"

namespace Wheel
{
    namespace Engine
    {
        class Scene
        {
        public:
            Scene()
            {
                m_EntityManager = new EntityManager();
                m_ComponentManager = new ComponentManager();
                m_SystemManager = new SystemManager();
            }
            ~Scene()
            {
                delete m_EntityManager;
                delete m_ComponentManager;
                delete m_SystemManager;
            }

            void Update(float deltaTime)
            {
                m_SystemManager->Update(deltaTime);
            }

            /**
            * @brief  Register all components before creating any entities. Built-in components do not have to be registered
            */
            template <typename T>
            void RegisterComponentType()
            {
                m_ComponentManager->RegisterComponent<T>();
            }

            /**
            * @brief  Register all systems before creating any entities. Built-in systems do not have to be registered
            */
            template<typename T>
            T* RegisterSystem(const Description& a_Description)
            {
                return m_SystemManager->RegisterSystem<T>(std::move(a_Description));
            }

            /**
             * @brief Add a component with default values. You can later update the component via GetComponent<T>(entityId)
             */
            template <typename T>
            T& AddComponent(uint32_t a_Entity)
            {
                m_EntityManager->GetEntityDescription(a_Entity).AddComponentType(m_ComponentManager->GetDescription<T>().GetAsBitset());
                T& component = m_ComponentManager->AddComponent<T>(a_Entity);
                m_SystemManager->AddEntityWithComponent(a_Entity, m_ComponentManager->GetComponentBitset<T>());
                return component;
            }

            template <typename T>
            void RemoveComponent(uint32_t a_Entity)
            {
                m_ComponentManager->GetComponentPool<T>()->RemoveComponent(a_Entity);
                m_EntityManager->GetEntityDescription(a_Entity).RemoveComponentType(m_ComponentManager->GetDescription<T>().GetAsBitset());
            }

            template <typename T>
            T& GetComponent(uint32_t a_Entity)
            {
                return m_ComponentManager->GetComponentPool<T>()->GetComponent(a_Entity);
            }

            template <typename T>
            bool HasComponent(uint32_t a_Entity)
            {
                Description& entityDescription = m_EntityManager->GetEntityDescription(a_Entity);
                Description& componentBitset = m_ComponentManager->GetDescription<T>();
                return entityDescription.HasComponentType(componentBitset);
            }

            /**
             *
             * @param a_AddToWorld Set this to false if you do not want to render the entity.
             * @return The ID of the created entity. Without you CANNOT interact with the entity in any way, so make sure to store it somewhere if you want to use it.
             */
            uint32_t AddEntity(bool a_AddToWorld = true) {
                uint32_t temp = m_EntityManager->CreateEntity();
                return temp;
            }

            void RemoveEntity(uint32_t a_Entity) {
                m_EntityManager->DestroyEntity(a_Entity);
                m_ComponentManager->EntityDestroyed(a_Entity);
                m_SystemManager->RemoveEntityWithComponent(a_Entity, m_EntityManager->GetEntityDescription(a_Entity));

            }
            size_t GetNumberOfEntities()
            {
                return m_EntityManager->GetEntityCount();
            }

            template <typename T>
            std::unordered_map<uint32_t,T*> GetComponents()
            {
                auto components = m_ComponentManager->GetComponentPool<T>()->GetComponents();
                return components;
            }

            uint8_t GetNumberOfComponentTypes()
            {
                return m_ComponentManager->GetNumberOfComponentTypes();
            }
            template <typename T>
            Description GetComponentDescription()
            {
                return m_ComponentManager->GetComponentBitset<T>();
            }
        private:
            EntityManager* m_EntityManager;
            SystemManager* m_SystemManager;
            ComponentManager* m_ComponentManager;
        };
    }
} // Wheel
