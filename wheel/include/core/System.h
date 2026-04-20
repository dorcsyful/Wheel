#pragma once
#include <cstdint>

#include "Description.h"
#include "ComponentPool.h"
namespace Wheel
{
    namespace Engine
    {
        class System
        {
        public:
            virtual ~System() = default;
            void AddEntityWithComponent(uint32_t a_Entity, Description& description)
            {
                if (m_Description.HasComponentType(description))
                {
                    m_EntityIDs.push_back(a_Entity);
                }
            }
            void RemoveEntityWithComponent(uint32_t a_Entity, Description& description)
            {
                if (m_Description.HasComponentType(description))
                    m_EntityIDs.erase(std::find(m_EntityIDs.begin(), m_EntityIDs.end(), a_Entity));
            }
            System(Description a_Description) : m_Description(std::move(a_Description)) {}


            virtual void Update(float deltaTime) = 0;
        protected:
            std::vector<uint32_t> m_EntityIDs;
            Description m_Description;
        };
    }
}