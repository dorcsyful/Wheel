#pragma once
#include <cstdint>
#include <iostream>
#include <vector>

#include "events/CollisionDispatcher.h"
#include "events/Events.h"
#include "SystemManager.h"
#include "ComponentManager.h"
#include "EntityManager.h"

namespace Wheel
{
    namespace Core
    {
        class Scene
        {
        public:
            Scene()
            {
                EventSystem::CollisionDispatcher::get();
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
#ifdef DEBUG_BUILD
                Debug::Debugger::get().RegisterComponentDescriptor<T>(m_ComponentManager->GetDescription<T>());
#endif
            }

            /**
            * @brief  Register all systems before creating any entities. Built-in systems do not have to be registered
            */
            template<typename T>
            T* RegisterSystem(const Description& a_Description)
            {
                T* system = m_SystemManager->RegisterSystem<T>(std::move(a_Description));
                if (system) system->SetScene(this);
                return system;
            }

            /**
             * @brief Add a component with default values. You can later update the component via GetComponent<T>(entityId)
             */
            template <typename T>
            T& AddComponent(uint32_t a_Entity)
            {
                m_EntityManager->GetEntityDescription(a_Entity).AddComponentType(m_ComponentManager->GetDescription<T>().GetAsBitset());
                T& component = m_ComponentManager->AddComponent<T>(a_Entity);
                m_SystemManager->AddEntityWithComponent(a_Entity, m_EntityManager->GetEntityDescription(a_Entity));
                EventSystem::EventBus::Publish(Events::ComponentAddedEvent<T>{ a_Entity });
                return component;
            }

            template <typename T>
            void RemoveComponent(uint32_t a_Entity)
            {
                // Publish before removal so handlers can still read the component if needed.
                EventSystem::EventBus::Publish(Events::ComponentRemovedEvent<T>{ a_Entity });
                m_ComponentManager->GetComponentPool<T>()->RemoveComponent(a_Entity);
                m_EntityManager->GetEntityDescription(a_Entity).RemoveComponentType(m_ComponentManager->GetDescription<T>().GetAsBitset());
            }

            template <typename T>
            T& GetComponent(uint32_t a_Entity)
            {
                return m_ComponentManager->GetComponentPool<T>()->GetComponent(a_Entity);
            }

            void* GetComponentRaw(uint32_t a_Entity, const Description& a_Description)
            {
                return m_ComponentManager->GetComponentRaw(a_Entity, a_Description);
            }

            const std::array<Description, MAX_ENTITIES>* GetEntityDescriptions() const
            {
                return m_EntityManager->GetEntityDescriptions();
            }

            template<typename T>
            T* GetSystem()
            {
                return m_SystemManager->GetSystem<T>();
            }

            template <typename T>
            bool HasComponent(uint32_t a_Entity)
            {
                Description& entityDescription = m_EntityManager->GetEntityDescription(a_Entity);
                Description& componentBitset = m_ComponentManager->GetDescription<T>();
                return entityDescription.HasComponentType(componentBitset);
            }

            /**
             * @return The ID of the created entity. Without you CANNOT interact with the entity in any way, so make sure to store it somewhere if you want to use it.
             */
            uint32_t AddEntity() {
                uint32_t temp = m_EntityManager->CreateEntity();
                return temp;
            }

            void RemoveEntity(uint32_t a_Entity) {
                EventSystem::EventBus::Publish(Events::EntityDestroyed{ a_Entity });
                m_ComponentManager->EntityDestroyed(a_Entity);
                m_SystemManager->RemoveEntityWithComponent(a_Entity, m_EntityManager->GetEntityDescription(a_Entity));
                m_EntityManager->DestroyEntity(a_Entity);
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

            // Returns the raw pool. Note that this does NOT get you the list of components
            template <typename T>
            ComponentPool<T>* GetComponentPool()
            {
                return m_ComponentManager->GetComponentPool<T>();
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
            Description GetComponentDescription(const std::string& a_Name)
            {
                return m_ComponentManager->GetDescriptionByName(a_Name);
            }
            std::string GetComponentName(const Description& a_Description)
            {
                return m_ComponentManager->GetNameByDescription(a_Description);
            }

            std::vector<std::string> GetEntityComponentNames(uint32_t a_Entity)
            {
                std::vector<std::string> names;
                auto& entityBitset = m_EntityManager->GetEntityDescription(a_Entity).GetAsBitset();
                uint8_t numTypes = m_ComponentManager->GetNumberOfComponentTypes();
                for (size_t i = 0; i < numTypes; ++i)
                {
                    if (!entityBitset[i]) continue;
                    std::bitset<MAX_COMPONENT_TYPES> singleBit;
                    singleBit[i] = true;
                    Description singleDesc;
                    singleDesc.AddComponentType(singleBit);
                    std::string name = m_ComponentManager->GetNameByDescription(singleDesc);
                    if (!name.empty())
                        names.push_back(name);
                }
                return names;
            }

            /**
             * Removes all entities, components and systems from the scene. Use this if you want to reset the scene without destroying it and creating a new one. Note that you still have to re-register all components and systems after calling this function, as well as re-creating all entities.
             */
            void Reset()
            {
                delete m_ComponentManager;
                delete m_SystemManager;
                delete m_EntityManager;
                m_EntityManager = new EntityManager();
                m_ComponentManager = new ComponentManager();
                m_SystemManager = new SystemManager();
            }


        private:
            EntityManager* m_EntityManager;
            SystemManager* m_SystemManager;
            ComponentManager* m_ComponentManager;
        };
    }
} // Wheel
