#pragma once
#include <unordered_set>

#include "EventBus.h"
#include "Events.h"
namespace Wheel
{
    namespace EventSystem
    {
        class CollisionDispatcher
        {
        public:
            CollisionDispatcher(const CollisionDispatcher& obj) = delete;
            void operator=(const CollisionDispatcher&)  = delete;
            ~CollisionDispatcher() = default;
            static CollisionDispatcher& get() {  static CollisionDispatcher instance; return instance;}
            CollisionDispatcher()
            {
                EventBus::Subscribe<Events::EntityDestroyed>(
                [this](const  Events::EntityDestroyed& event)
                {
                    uint32_t entityId = event.entityId;
                    std::vector<uint64_t> enterPairsToRemove;
                    std::vector<uint64_t> exitPairsToRemove;
                    std::vector<uint64_t> currentPairsToRemove;
                    int maximum = std::max(m_EnterHandlers.size(), std::max(m_ExitHandlers.size(), m_ActivePairs.size()));
                    for (int i = 0; i < maximum; i++)
                    {
                        if (i < m_EnterHandlers.size())
                        {
                            auto it = std::next(m_EnterHandlers.begin(), i);
                            uint32_t a,b;
                            Unkey(it->first, a, b);
                            if (a == entityId || b == entityId)
                            {
                                enterPairsToRemove.push_back(it->first);
                            }
                        }
                        if (i < m_ExitHandlers.size())
                        {
                            auto it = std::next(m_ExitHandlers.begin(), i);
                            uint32_t a,b;
                            Unkey(it->first, a, b);
                            if (a == entityId || b == entityId)
                            {
                                exitPairsToRemove.push_back(it->first);
                            }
                        }
                        if (i < m_ActivePairs.size())
                        {
                            auto it = std::next(m_ActivePairs.begin(), i);
                            uint32_t a,b;
                            Unkey(*it, a, b);
                            if (a == entityId || b == entityId)
                            {
                                currentPairsToRemove.push_back(*it);
                            }
                        }
                    }
                    maximum = std::max(enterPairsToRemove.size(), std::max(exitPairsToRemove.size(), currentPairsToRemove.size()));
                    for (int i = 0; i < maximum; i++)
                    {
                        if (i < enterPairsToRemove.size()) m_EnterHandlers.erase(enterPairsToRemove[i]);
                        if (i < exitPairsToRemove.size()) m_ExitHandlers.erase(exitPairsToRemove[i]);
                        if (i < currentPairsToRemove.size()) m_ActivePairs.erase(currentPairsToRemove[i]);
                    }

                }
                );

                EventBus::Subscribe<Events::CollisionResultsFinished>(
                    [this](const Events::CollisionResultsFinished& event)
                    {
                        std::unordered_set<uint64_t> lastActivePairs = std::move(m_ActivePairs);
                        m_ActivePairs.clear();
                        for (int i = 0; i < event.ManifoldPtr->size(); i++)
                        {
                            const auto& m = (*event.ManifoldPtr)[i];
                            uint64_t pairKey = Key(m.collider1, m.collider2);
                            m_ActivePairs.insert(pairKey);
                            if (lastActivePairs.erase(pairKey) == 0)
                            {
                                auto it = m_EnterHandlers.find(pairKey);
                                if (it != m_EnterHandlers.end())
                                {
                                    for (auto& cb : it->second)
                                    {
                                        cb(m);
                                    }
                                }
                            }
                        }
                        for (auto& pairKey : lastActivePairs)
                        {
                            uint32_t a,b;
                            Unkey(pairKey, a, b);
                            EventBus::Publish(Events::CollisionExitEvent(a,b));
                        }
                    });
            }
            void OnCollision(uint32_t a, uint32_t b,
                 std::function<void(const Engine::Collision::Collision2DManifold&)> cb) {
                m_EnterHandlers[Key(a, b)].push_back(std::move(cb));
            }

            void OnCollisionExit(uint32_t a, uint32_t b,
                 std::function<void(uint32_t a, uint32_t b)> cb) {
                m_ExitHandlers[Key(a, b)].push_back(std::move(cb));
            }

        private:
            //combine A and B
            static uint64_t Key(uint32_t a, uint32_t b) {
                if (a > b) std::swap(a, b);
                return (static_cast<uint64_t>(a) << 32) | b;
            }
            static void Unkey(uint64_t key, uint32_t& a, uint32_t& b) {
                a = static_cast<uint32_t>(key >> 32);   // high 32 bits
                b = static_cast<uint32_t>(key);          // low 32 bits (top half truncated away)
            }
            std::unordered_set<uint64_t> m_ActivePairs {};
            std::unordered_map<uint64_t, std::vector<std::function<void(const Engine::Collision::Collision2DManifold&)>>> m_EnterHandlers;
            std::unordered_map<uint64_t, std::vector<std::function<void(uint32_t a, uint32_t b)>>> m_ExitHandlers;
        };
    }
}