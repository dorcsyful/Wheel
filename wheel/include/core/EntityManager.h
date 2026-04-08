#pragma once
#include <cassert>
#include <cstdint>
#include <queue>
#include <array>
#include "Description.h"
#include "Globals.h"

namespace Wheel
{
    namespace Engine
    {
        class EntityManager
        {
        public:
            EntityManager()
            {
                m_EntityCount = 0;
                for (uint32_t entityID = 0; entityID < MAX_ENTITIES; ++entityID)
                {
                    m_AvailableEntities.push(entityID);
                    m_EntityDescriptions[entityID] = Description();
                }
            }
            ~EntityManager() = default;

            uint32_t CreateEntity()
            {
                assert(m_EntityCount < MAX_ENTITIES && "Too many entities in existence.");
                uint32_t entityID;
                entityID = m_AvailableEntities.front();
                m_AvailableEntities.pop();
                m_EntityCount++;
                return entityID;
            }
            void DestroyEntity(uint32_t a_EntityID) {
                assert(a_EntityID < MAX_ENTITIES && "Entity ID out of range.");
                m_EntityDescriptions[a_EntityID].Reset();
                m_AvailableEntities.push(a_EntityID);
                --m_EntityCount;
            }

            uint32_t GetEntityCount() { return m_EntityCount; }

            Description& GetEntityDescription(uint32_t a_EntityID)
            {
                return m_EntityDescriptions[a_EntityID];
            }

        private:
            std::array<Description, MAX_ENTITIES> m_EntityDescriptions;
            std::queue<uint32_t> m_AvailableEntities;
            uint32_t m_EntityCount;
        };
    }
}

