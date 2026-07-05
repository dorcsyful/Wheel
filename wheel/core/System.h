#pragma once
#include <algorithm>
#include <cstdint>

#include "Description.h"
#include "ComponentPool.h"

namespace Wheel { namespace Core { class Scene; } }

namespace Wheel
{
    namespace Core
    {
        class System
        {
        public:
            virtual ~System() = default;
            void AddEntityWithComponent(uint32_t a_Entity, Description& description)
            {
                //TODO: find a way to remove search
                if (description.HasComponentType(m_Description) &&
                    std::find(m_EntityIDs.begin(), m_EntityIDs.end(), a_Entity) == m_EntityIDs.end())
                {
                    m_EntityIDs.push_back(a_Entity);
                }
            }
            void RemoveEntityWithComponent(uint32_t a_Entity, Description& description)
            {
                //TODO: find a way to remove search
                if (description.HasComponentType(m_Description)&&
                    std::find(m_EntityIDs.begin(), m_EntityIDs.end(), a_Entity) != m_EntityIDs.end())
                    m_EntityIDs.erase(std::find(m_EntityIDs.begin(), m_EntityIDs.end(), a_Entity));
            }
            System(Description a_Description) : m_Description(std::move(a_Description)) {}

            void SetScene(Scene* a_Scene) { m_Scene = a_Scene; }

            virtual void Update(float deltaTime) = 0;
        protected:
            Scene* m_Scene = nullptr;
            std::vector<uint32_t> m_EntityIDs;
            Description m_Description;
        };
    }
}