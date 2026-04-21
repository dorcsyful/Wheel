#include "gtest/gtest.h"
#include "EventBus.h"

using namespace Wheel::EventSystem;

// --- Mock events ---

struct CountEvent {
    int value = 0;
};

struct StringEvent {
    std::string message;
};

struct TypeAEvent {};
struct TypeBEvent {};

// --- Basic publish/subscribe ---

TEST(EventBus, SubscriberIsCalledOnPublish) {
    SubscriptionToken token;
    bool called = false;
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { called = true; }, token);
    EventBus::Publish(CountEvent{});
    EXPECT_TRUE(called);
}

TEST(EventBus, SubscriberReceivesCorrectEventData) {
    SubscriptionToken token;
    int received = -1;
    EventBus::Subscribe<CountEvent>([&](const CountEvent& e) { received = e.value; }, token);
    EventBus::Publish(CountEvent{42});
    EXPECT_EQ(received, 42);
}

TEST(EventBus, MultipleSubscribersAllReceiveEvent) {
    SubscriptionToken token;
    int callCount = 0;
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { callCount++; }, token);
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { callCount++; }, token);
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { callCount++; }, token);
    EventBus::Publish(CountEvent{});
    EXPECT_EQ(callCount, 3);
}

TEST(EventBus, PublishOnlyFiresCorrectEventType) {
    SubscriptionToken token;
    bool aCalled = false;
    bool bCalled = false;
    EventBus::Subscribe<TypeAEvent>([&](const TypeAEvent&) { aCalled = true; }, token);
    EventBus::Subscribe<TypeBEvent>([&](const TypeBEvent&) { bCalled = true; }, token);
    EventBus::Publish(TypeAEvent{});
    EXPECT_TRUE(aCalled);
    EXPECT_FALSE(bCalled);
}

TEST(EventBus, SubscriberNotCalledWhenDifferentEventPublished) {
    SubscriptionToken token;
    bool called = false;
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { called = true; }, token);
    EventBus::Publish(StringEvent{"hello"});
    EXPECT_FALSE(called);
}

// --- Token-based unsubscription ---

TEST(EventBus, HandlerNotCalledAfterTokenDestroyed) {
    bool called = false;
    {
        SubscriptionToken token;
        EventBus::Subscribe<CountEvent>([&](const CountEvent&) { called = true; }, token);
    } // token destroyed here -> auto-unsubscribe
    EventBus::Publish(CountEvent{});
    EXPECT_FALSE(called);
}

TEST(EventBus, HandlerCalledBeforeTokenDestroyedButNotAfter) {
    SubscriptionToken token;
    int callCount = 0;
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { callCount++; }, token);
    EventBus::Publish(CountEvent{});
    EXPECT_EQ(callCount, 1);
    token.~SubscriptionToken();
    new (&token) SubscriptionToken(); // reset to empty token to avoid double-destroy
    EventBus::Publish(CountEvent{});
    EXPECT_EQ(callCount, 1);
}

TEST(EventBus, MultipleHandlersUnsubscribeIndependently) {
    SubscriptionToken tokenA;
    int aCount = 0;
    int bCount = 0;
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { aCount++; }, tokenA);
    {
        SubscriptionToken tokenB;
        EventBus::Subscribe<CountEvent>([&](const CountEvent&) { bCount++; }, tokenB);
        EventBus::Publish(CountEvent{});
        EXPECT_EQ(aCount, 1);
        EXPECT_EQ(bCount, 1);
    } // tokenB destroyed
    EventBus::Publish(CountEvent{});
    EXPECT_EQ(aCount, 2);
    EXPECT_EQ(bCount, 1); // B no longer receives events
}

// --- Filtered subscription ---

TEST(EventBus, FilteredSubscriberCalledWhenFilterPasses) {
    SubscriptionToken token;
    bool called = false;
    EventBus::Subscribe<CountEvent>(
        [](const CountEvent& e) { return e.value > 10; },
        [&](const CountEvent&) { called = true; },
        token
    );
    EventBus::Publish(CountEvent{20});
    EXPECT_TRUE(called);
}

TEST(EventBus, FilteredSubscriberNotCalledWhenFilterFails) {
    SubscriptionToken token;
    bool called = false;
    EventBus::Subscribe<CountEvent>(
        [](const CountEvent& e) { return e.value > 10; },
        [&](const CountEvent&) { called = true; },
        token
    );
    EventBus::Publish(CountEvent{5});
    EXPECT_FALSE(called);
}

TEST(EventBus, FilteredAndUnfilteredSubscribersCoexist) {
    SubscriptionToken token;
    int filteredCount = 0;
    int unfilteredCount = 0;
    EventBus::Subscribe<CountEvent>(
        [](const CountEvent& e) { return e.value > 10; },
        [&](const CountEvent&) { filteredCount++; },
        token
    );
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { unfilteredCount++; }, token);

    EventBus::Publish(CountEvent{5});
    EXPECT_EQ(filteredCount, 0);
    EXPECT_EQ(unfilteredCount, 1);

    EventBus::Publish(CountEvent{20});
    EXPECT_EQ(filteredCount, 1);
    EXPECT_EQ(unfilteredCount, 2);
}

TEST(EventBus, FilterReceivesCorrectEventData) {
    SubscriptionToken token;
    std::string captured;
    EventBus::Subscribe<StringEvent>(
        [](const StringEvent& e) { return e.message == "ping"; },
        [&](const StringEvent& e) { captured = e.message; },
        token
    );
    EventBus::Publish(StringEvent{"pong"});
    EXPECT_TRUE(captured.empty());
    EventBus::Publish(StringEvent{"ping"});
    EXPECT_EQ(captured, "ping");
}

// --- Multiple publishes ---

TEST(EventBus, HandlerCalledOncePerPublish) {
    SubscriptionToken token;
    int callCount = 0;
    EventBus::Subscribe<CountEvent>([&](const CountEvent&) { callCount++; }, token);
    EventBus::Publish(CountEvent{});
    EventBus::Publish(CountEvent{});
    EventBus::Publish(CountEvent{});
    EXPECT_EQ(callCount, 3);
}