#pragma once
// EventBus.h
#include <algorithm>
#include <functional>
#include <vector>
#include <unordered_map>
#include <typeindex>

#include "SubscriptionToken.h"

//very basic event system

namespace Wheel
{
    namespace EventSystem
    {
        template <typename TEvent>
        struct Handler {
            int id;
            std::function<void(const TEvent&)> callback;
            std::function<bool(const TEvent&)> filter; // nullptr = no filter, always fires
        };

        class EventBus {
        public:

            template <typename TEvent, typename TCallback>
            static void Subscribe(TCallback&& callback)
            {
                GetHandlers<TEvent>().push_back({ s_NextId++, std::forward<TCallback>(callback), nullptr });
            }

            /**
             * @param token Holds the unsubscribe lambda. Must be a global variable or a class member!
             */
            template <typename TEvent, typename TCallback>
            static void Subscribe(TCallback&& callback, SubscriptionToken& token)
            {
                int id = s_NextId++;
                GetHandlers<TEvent>().push_back({ id, std::forward<TCallback>(callback), nullptr });
                token.m_Unsubscribers.push_back([id]() {
                    auto& handlers = GetHandlers<TEvent>();
                    handlers.erase(
                        std::remove_if(handlers.begin(), handlers.end(),
                            [id](const Handler<TEvent>& h) { return h.id == id; }),
                        handlers.end());
                });
            }

            template <typename TEvent, typename TFilter, typename TCallback>
            static void Subscribe(TFilter&& filter, TCallback&& callback, SubscriptionToken& token)
            {
                int id = s_NextId++;
                GetHandlers<TEvent>().push_back({ id, std::forward<TCallback>(callback), std::forward<TFilter>(filter) });
                token.m_Unsubscribers.push_back([id]() {
                    auto& handlers = GetHandlers<TEvent>();
                    handlers.erase(
                        std::remove_if(handlers.begin(), handlers.end(),
                            [id](const Handler<TEvent>& h) { return h.id == id; }),
                        handlers.end());
                });
            }

            template <typename TEvent>
            static void Publish(const TEvent& event) {
                for (auto& handler : GetHandlers<TEvent>()) {
                    if (!handler.filter || handler.filter(event))
                        handler.callback(event);
                }
            }


        private:

            inline static int s_NextId = 0;

            template <typename TEvent>
            static std::vector<Handler<TEvent>>& GetHandlers() {
                static std::vector<Handler<TEvent>> handlers;
                return handlers;
            }
        };

    }
}