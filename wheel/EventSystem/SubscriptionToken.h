#pragma once
#include <functional>
#include <vector>

// A token that unsubscribes automatically when destroyed
namespace Wheel
{
    namespace EventSystem
    {
        struct SubscriptionToken
        {
            ~SubscriptionToken() {
                for (auto& unsub : m_Unsubscribers)
                    unsub();
            }

            // Non-copyable, movable
            SubscriptionToken() = default;
            SubscriptionToken(const SubscriptionToken&) = delete;
            SubscriptionToken& operator=(const SubscriptionToken&) = delete;
            SubscriptionToken(SubscriptionToken&&) = default;
            SubscriptionToken& operator=(SubscriptionToken&&) = default;

            std::vector<std::function<void()>> m_Unsubscribers;
        };
    }
}
