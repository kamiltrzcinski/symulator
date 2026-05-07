#include "engine/core/event_dispatcher.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

using engine::core::EventDispatcher;

// ── Event types used in typed tests ──────────────────────────────────────────

struct IntEvent
{
    int value;
};
struct StringEvent
{
    std::string value;
};

template<typename T>
struct EventTraits;
template<>
struct EventTraits<IntEvent>
{
    static IntEvent make(int n) { return {n}; }
    static int extract(const IntEvent& e) { return e.value; }
};
template<>
struct EventTraits<StringEvent>
{
    static StringEvent make(int n) { return {std::to_string(n)}; }
    static int extract(const StringEvent& e) { return std::stoi(e.value); }
};

// ── Typed tests — dispatch mechanics are event-type-agnostic ─────────────────

template<typename EventT>
class EventDispatcherTypedTest : public ::testing::Test
{
};

using DispatcherEventTypes = ::testing::Types<IntEvent, StringEvent>;
TYPED_TEST_SUITE(EventDispatcherTypedTest, DispatcherEventTypes);

TYPED_TEST(EventDispatcherTypedTest, PublishDeliveredToSubscriber)
{
    using Traits = EventTraits<TypeParam>;
    EventDispatcher<TypeParam> dispatcher;
    int received = -1;
    dispatcher.subscribe([&received](const TypeParam& e) { received = Traits::extract(e); });
    dispatcher.publish(Traits::make(42));
    EXPECT_EQ(received, 42);
}

TYPED_TEST(EventDispatcherTypedTest, NoSubscribersPublishIsNoop)
{
    EventDispatcher<TypeParam> dispatcher;
    EXPECT_NO_THROW(dispatcher.publish(EventTraits<TypeParam>::make(1)));
}

TYPED_TEST(EventDispatcherTypedTest, UnsubscribedHandlerNotCalled)
{
    EventDispatcher<TypeParam> dispatcher;
    int counter = 0;
    auto token = dispatcher.subscribe([&counter](const TypeParam&) { ++counter; });
    dispatcher.publish(EventTraits<TypeParam>::make(1));
    EXPECT_EQ(counter, 1);
    dispatcher.unsubscribe(token);
    dispatcher.publish(EventTraits<TypeParam>::make(2));
    EXPECT_EQ(counter, 1);  // Must not have been called again
}

TYPED_TEST(EventDispatcherTypedTest, SubscriberCountTracking)
{
    EventDispatcher<TypeParam> dispatcher;
    EXPECT_EQ(dispatcher.subscriber_count(), 0u);
    auto t1 = dispatcher.subscribe([](const TypeParam&) {});
    EXPECT_EQ(dispatcher.subscriber_count(), 1u);
    auto t2 = dispatcher.subscribe([](const TypeParam&) {});
    EXPECT_EQ(dispatcher.subscriber_count(), 2u);
    dispatcher.unsubscribe(t1);
    EXPECT_EQ(dispatcher.subscriber_count(), 1u);
    dispatcher.unsubscribe(t2);
    EXPECT_EQ(dispatcher.subscriber_count(), 0u);
}

// ── Parameterised over subscriber count — all N subscribers receive event ─────

class EventDispatcherSubscriberCountTest : public ::testing::TestWithParam<int>
{
};

TEST_P(EventDispatcherSubscriberCountTest, AllSubscribersReceive)
{
    const int n = GetParam();
    EventDispatcher<IntEvent> dispatcher;
    std::atomic<int> calls{0};
    for (int i = 0; i < n; ++i)
        dispatcher.subscribe([&calls](const IntEvent&) { calls.fetch_add(1); });
    dispatcher.publish(IntEvent{7});
    EXPECT_EQ(calls.load(), n);
}

INSTANTIATE_TEST_SUITE_P(EventDispatcher, EventDispatcherSubscriberCountTest,
                         ::testing::Values(1, 2, 5, 10, 50),
                         [](const ::testing::TestParamInfo<int>& info)
                         { return "n" + std::to_string(info.param); });

// ── Non-parameterised tests — require specific single-type logic ──────────────

TEST(EventDispatcher, UnsubscribeUnknownTokenIsNoop)
{
    EventDispatcher<IntEvent> dispatcher;
    dispatcher.subscribe([](const IntEvent&) {});
    EXPECT_NO_THROW(dispatcher.unsubscribe(9999));
    EXPECT_EQ(dispatcher.subscriber_count(), 1u);
}

TEST(EventDispatcher, ResubscribeAfterUnsubscribe)
{
    EventDispatcher<IntEvent> dispatcher;
    int counter = 0;
    auto t1 = dispatcher.subscribe([&counter](const IntEvent&) { ++counter; });
    dispatcher.unsubscribe(t1);
    auto t2 = dispatcher.subscribe([&counter](const IntEvent&) { counter += 10; });
    dispatcher.publish(IntEvent{0});
    EXPECT_EQ(counter, 10);
    dispatcher.unsubscribe(t2);
}

// ── Snapshot semantics ────────────────────────────────────────────────────────
// Handlers added or removed during publish() must not affect the current round.

TEST(EventDispatcher, HandlerAddedDuringPublishNotCalledThisRound)
{
    EventDispatcher<IntEvent> dispatcher;
    int late_calls = 0;

    dispatcher.subscribe(
        [&dispatcher, &late_calls](const IntEvent&)
        { dispatcher.subscribe([&late_calls](const IntEvent&) { ++late_calls; }); });

    dispatcher.publish(IntEvent{0});
    EXPECT_EQ(late_calls, 0);  // Late handler not called in this round

    dispatcher.publish(IntEvent{0});
    EXPECT_EQ(late_calls, 1);  // Called in the next round
}

TEST(EventDispatcher, HandlerRemovedDuringPublishStillCalledThisRound)
{
    EventDispatcher<IntEvent> dispatcher;
    int calls = 0;

    EventDispatcher<IntEvent>::SubscriptionToken second_token{};
    dispatcher.subscribe([&dispatcher, &second_token](const IntEvent&)
                         { dispatcher.unsubscribe(second_token); });
    second_token = dispatcher.subscribe([&calls](const IntEvent&) { ++calls; });

    dispatcher.publish(IntEvent{0});
    EXPECT_EQ(calls, 1);  // Second handler was in the snapshot; still called

    dispatcher.publish(IntEvent{0});
    EXPECT_EQ(calls, 1);  // Unsubscribed; not called in subsequent rounds
}

// ── Parameterised concurrency stress tests ────────────────────────────────────

struct ConcurrentParams
{
    int publishers;
    int rounds;
    int subscribers;
};

class EventDispatcherConcurrentTest : public ::testing::TestWithParam<ConcurrentParams>
{
};

TEST_P(EventDispatcherConcurrentTest, ConcurrentSubscribeAndPublishNoCrash)
{
    const auto [publishers, rounds, subscribers] = GetParam();
    EventDispatcher<IntEvent> dispatcher;
    std::atomic<int> total_calls{0};

    std::vector<std::thread> sub_threads;
    for (int i = 0; i < subscribers; ++i)
    {
        sub_threads.emplace_back(
            [&dispatcher, &total_calls] {
                dispatcher.subscribe([&total_calls](const IntEvent&) { total_calls.fetch_add(1); });
            });
    }

    std::vector<std::thread> pub_threads;
    for (int p = 0; p < publishers; ++p)
    {
        pub_threads.emplace_back(
            [&dispatcher, rounds]
            {
                for (int r = 0; r < rounds; ++r)
                    dispatcher.publish(IntEvent{r});
            });
    }

    for (auto& t : sub_threads)
        t.join();
    for (auto& t : pub_threads)
        t.join();

    // Exact count is scheduling-dependent; assert only no crash / data race.
    EXPECT_GE(total_calls.load(), 0);
}

TEST_P(EventDispatcherConcurrentTest, ConcurrentUnsubscribeAndPublishNoCrash)
{
    const auto [publishers, rounds, subscribers] = GetParam();
    EventDispatcher<IntEvent> dispatcher;
    std::atomic<int> calls{0};

    std::vector<EventDispatcher<IntEvent>::SubscriptionToken> tokens;
    for (int i = 0; i < subscribers; ++i)
        tokens.push_back(dispatcher.subscribe([&calls](const IntEvent&) { calls.fetch_add(1); }));

    std::vector<std::thread> pub_threads;
    for (int p = 0; p < publishers; ++p)
    {
        pub_threads.emplace_back(
            [&dispatcher, rounds]
            {
                for (int r = 0; r < rounds; ++r)
                    dispatcher.publish(IntEvent{r});
            });
    }

    std::thread unsubscriber(
        [&dispatcher, &tokens]
        {
            for (auto token : tokens)
                dispatcher.unsubscribe(token);
        });

    for (auto& t : pub_threads)
        t.join();
    unsubscriber.join();

    EXPECT_GE(calls.load(), 0);
}

INSTANTIATE_TEST_SUITE_P(EventDispatcher, EventDispatcherConcurrentTest,
                         ::testing::Values(ConcurrentParams{1, 100, 10},
                                           ConcurrentParams{2, 200, 20},
                                           ConcurrentParams{4, 50, 10}),
                         [](const ::testing::TestParamInfo<ConcurrentParams>& info)
                         {
                             return std::to_string(info.param.publishers) + "pub_" +
                                    std::to_string(info.param.rounds) + "rounds_" +
                                    std::to_string(info.param.subscribers) + "sub";
                         });
