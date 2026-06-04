#include "engine/core/event_queue.hpp"

#include <gtest/gtest.h>

#include <tests/common/param_test_helpers.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using engine::core::EventQueue;

// ── Helpers ───────────────────────────────────────────────────────────────────

template<typename T>
T queue_value(int n);
template<>
int queue_value<int>(int n)
{
    return n;
}
template<>
std::string queue_value<std::string>(int n)
{
    return std::to_string(n);
}

// ── Typed tests — basic behaviour over multiple element types ────────────────
// Covers: push/pop, FIFO, size, empty, close/drain, push-after-close, is_closed.

template<typename T>
class EventQueueTypedTest : public ::testing::Test
{
};

using ElementTypes = ::testing::Types<int, std::string>;
TYPED_TEST_SUITE(EventQueueTypedTest, ElementTypes);

TYPED_TEST(EventQueueTypedTest, PushAndTryPop)
{
    EventQueue<TypeParam> q;
    q.push(queue_value<TypeParam>(42));
    auto result = q.try_pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, queue_value<TypeParam>(42));
}

TYPED_TEST(EventQueueTypedTest, TryPopEmptyReturnsNullopt)
{
    EventQueue<TypeParam> q;
    EXPECT_FALSE(q.try_pop().has_value());
}

TYPED_TEST(EventQueueTypedTest, PreservesInsertionOrder)
{
    EventQueue<TypeParam> q;
    q.push(queue_value<TypeParam>(1));
    q.push(queue_value<TypeParam>(2));
    q.push(queue_value<TypeParam>(3));
    EXPECT_EQ(*q.try_pop(), queue_value<TypeParam>(1));
    EXPECT_EQ(*q.try_pop(), queue_value<TypeParam>(2));
    EXPECT_EQ(*q.try_pop(), queue_value<TypeParam>(3));
    EXPECT_FALSE(q.try_pop().has_value());
}

TYPED_TEST(EventQueueTypedTest, SizeTracking)
{
    EventQueue<TypeParam> q;
    EXPECT_EQ(q.size(), 0u);
    q.push(queue_value<TypeParam>(1));
    q.push(queue_value<TypeParam>(2));
    EXPECT_EQ(q.size(), 2u);
    q.try_pop();
    EXPECT_EQ(q.size(), 1u);
}

TYPED_TEST(EventQueueTypedTest, EmptyFlag)
{
    EventQueue<TypeParam> q;
    EXPECT_TRUE(q.empty());
    q.push(queue_value<TypeParam>(99));
    EXPECT_FALSE(q.empty());
    q.try_pop();
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(EventQueueTypedTest, CloseAfterPushDrainsNormally)
{
    EventQueue<TypeParam> q;
    q.push(queue_value<TypeParam>(7));
    q.push(queue_value<TypeParam>(8));
    q.close();
    EXPECT_EQ(*q.wait_and_pop(), queue_value<TypeParam>(7));
    EXPECT_EQ(*q.wait_and_pop(), queue_value<TypeParam>(8));
    EXPECT_FALSE(q.wait_and_pop().has_value());  // Drained after close
}

TYPED_TEST(EventQueueTypedTest, PushAfterCloseThrows)
{
    EventQueue<TypeParam> q;
    q.close();
    EXPECT_THROW(q.push(queue_value<TypeParam>(1)), std::runtime_error);
}

TYPED_TEST(EventQueueTypedTest, IsClosedFlag)
{
    EventQueue<TypeParam> q;
    EXPECT_FALSE(q.is_closed());
    q.close();
    EXPECT_TRUE(q.is_closed());
}

// ── Non-typed tests — require int or move-only type specifically ──────────────

TEST(EventQueue, CloseUnblocksWaitAndPop)
{
    EventQueue<int> q;
    std::thread consumer(
        [&q]
        {
            EXPECT_FALSE(q.wait_and_pop().has_value());  // Closed and empty -> nullopt
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    q.close();
    consumer.join();
}

TEST(EventQueue, MoveOnlyType)
{
    EventQueue<std::unique_ptr<int>> q;
    q.push(std::make_unique<int>(99));
    auto result = q.try_pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(**result, 99);
}

// ── Parameterised MPSC — different producer/item-count configurations ─────────

struct MpscParams
{
    const char* name;
    int producers;
    int items_per_producer;
};

class EventQueueMpscTest : public ::testing::TestWithParam<MpscParams>
{
};

TEST_P(EventQueueMpscTest, AllItemsDelivered)
{
    const auto& param = GetParam();
    const int producers = param.producers;
    const int items_per_producer = param.items_per_producer;
    const int total = producers * items_per_producer;

    EventQueue<int> q;
    std::vector<std::thread> threads;
    for (int p = 0; p < producers; ++p)
    {
        threads.emplace_back(
            [&q, p, items_per_producer]
            {
                for (int i = 0; i < items_per_producer; ++i)
                    q.push(p * items_per_producer + i);
            });
    }

    std::atomic<int> received{0};
    std::thread consumer(
        [&q, &received]
        {
            while (auto item = q.wait_and_pop())
                ++received;
        });

    for (auto& t : threads)
        t.join();
    q.close();
    consumer.join();

    EXPECT_EQ(received.load(), total);
}

INSTANTIATE_TEST_SUITE_P(EventQueue, EventQueueMpscTest,
                         ::testing::Values(MpscParams{"P1I2000", 1, 2000},
                                           MpscParams{"P2I1000", 2, 1000},
                                           MpscParams{"P4I500", 4, 500},
                                           MpscParams{"P8I250", 8, 250}),
                         tests::common::param_name<MpscParams>);
