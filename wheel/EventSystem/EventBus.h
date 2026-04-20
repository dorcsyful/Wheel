#pragma once
// EventBus.h
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
        };

        class EventBus {
        public:

            template <typename TEvent>
            static void Subscribe(std::function<void(const TEvent&)> callback,
                                  SubscriptionToken& token)
            {
                auto& handlers = GetHandlers<TEvent>();
                int id = s_NextId++;
                handlers.push_back({ id, callback });

                // When the token dies, this lambda runs and removes the handler
                token.m_Unsubscribers.push_back([id]() {
                    auto& h = GetHandlers<TEvent>();
                    h.erase(std::remove_if(h.begin(), h.end(),
                        [id](const Handler<TEvent>& handler) {
                            return handler.id == id;
                        }), h.end());
                });
            }

            template <typename TEvent>
            static void Publish(const TEvent& event) {
                for (auto& handler : GetHandlers<TEvent>())
                    handler.callback(event); // was just handler(event) before
            }


        private:

            static int s_NextId;

            template <typename TEvent>
            static std::vector<std::function<void(const TEvent&)>>& GetHandlers() {
                static std::vector<std::function<void(const TEvent&)>> handlers;
                return handlers;
            }
        };

    }
}