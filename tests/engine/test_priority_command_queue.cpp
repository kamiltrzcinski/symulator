#include "engine/core/priority_command_queue.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using engine::core::CommandPriority;
using engine::core::PriorityCommandQueue;

// ── Helpers ───────────────────────────────────────────────────────────────────

template<typename T>
T cmd_value(int n);
template<>
int cmd_value<int>(int n)
{
    return n;
}
template<>
std::string cmd_value<std::string>(int n)
{
    return std::to_string(n);
}

// ── Typed tests — push/pop/close over multiple element types ─────────────────

template<typename T>
class PriorityCommandQueueTypedTest : public ::testing::Test
{
};

using CmdElementTypes = ::testing::Types<int, std::string>;
TYPED_TEST_SUITE(PriorityCommandQueueTypedTest, CmdElementTypes);

TYPED_TEST(PriorityCommandQueueTypedTest, PushAndTryPop)
{
    PriorityCommandQueue<TypeParam> q;
    q.push(cmd_value<TypeParam>(1), CommandPriority::NORMAL);
    auto result = q.try_pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, cmd_value<TypeParam>(1));
}

TYPED_TEST(PriorityCommandQueueTypedTest, TryPopEmptyReturnsNullopt)
{
    PriorityCommandQueue<TypeParam> q;
    EXPECT_FALSE(q.try_pop().has_value());
}

TYPED_TEST(PriorityCommandQueueTypedTest, PushAfterCloseThrows)
{
    PriorityCommandQueue<TypeParam> q;
    q.close();
    EXPECT_THROW(q.push(cmd_value<TypeParam>(1), CommandPriority::NORMAL), std::runtime_error);
}

TYPED_TEST(PriorityCommandQueueTypedTest, IsClosedFlag)
{
    PriorityCommandQueue<TypeParam> q;
    EXPECT_FALSE(q.is_closed());
    q.close();
    EXPECT_TRUE(q.is_closed());
}

// ── Parameterised over CommandPriority — per-bucket correctness ───────────────
// Each test runs once per priority level to ensure no bucket is special-cased.

class PriorityCommandQueuePerPriorityTest : public ::testing::TestWithParam<CommandPriority>
{
};

TEST_P(PriorityCommandQueuePerPriorityTest, PushAndPopAtPriority)
{
    PriorityCommandQueue<int> q;
    q.push(42, GetParam());
    auto result = q.try_pop();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST_P(PriorityCommandQueuePerPriorityTest, FifoWithinSamePriority)
{
    PriorityCommandQueue<int> q;
    const auto p = GetParam();
    q.push(10, p);
    q.push(20, p);
    q.push(30, p);
    EXPECT_EQ(*q.try_pop(), 10);
    EXPECT_EQ(*q.try_pop(), 20);
    EXPECT_EQ(*q.try_pop(), 30);
    EXPECT_FALSE(q.try_pop().has_value());
}

INSTANTIATE_TEST_SUITE_P(AllPriorities, PriorityCommandQueuePerPriorityTest,
                         ::testing::Values(CommandPriority::EMERGENCY, CommandPriority::SAFETY,
                                           CommandPriority::NORMAL, CommandPriority::BACKGROUND),
                         [](const ::testing::TestParamInfo<CommandPriority>& info)
                         {
                             switch (info.param)
                             {
                                 case CommandPriority::EMERGENCY:
                                     return "EMERGENCY";
                                 case CommandPriority::SAFETY:
                                     return "SAFETY";
                                 case CommandPriority::NORMAL:
                                     return "NORMAL";
                                 case CommandPriority::BACKGROUND:
                                     return "BACKGROUND";
                             }
                             return "UNKNOWN";
                         });

// ── Non-parameterised tests — require specific multi-priority interaction ──────

TEST(PriorityCommandQueue, HigherPriorityDequeuesFirst)
{
    PriorityCommandQueue<std::string> q;
    q.push("normal", CommandPriority::NORMAL);
    q.push("background", CommandPriority::BACKGROUND);
    q.push("emergency", CommandPriority::EMERGENCY);
    q.push("safety", CommandPriority::SAFETY);
    EXPECT_EQ(*q.try_pop(), "emergency");
    EXPECT_EQ(*q.try_pop(), "safety");
    EXPECT_EQ(*q.try_pop(), "normal");
    EXPECT_EQ(*q.try_pop(), "background");
    EXPECT_FALSE(q.try_pop().has_value());
}

TEST(PriorityCommandQueue, EmergencyPreemptsAlreadyQueuedNormal)
{
    PriorityCommandQueue<int> q;
    q.push(1, CommandPriority::NORMAL);
    q.push(2, CommandPriority::NORMAL);
    q.push(99, CommandPriority::EMERGENCY);  // Added after normal items
    EXPECT_EQ(*q.try_pop(), 99);             // Must be first despite being pushed last
    EXPECT_EQ(*q.try_pop(), 1);
    EXPECT_EQ(*q.try_pop(), 2);
}

TEST(PriorityCommandQueue, SizeCountsAllBuckets)
{
    PriorityCommandQueue<int> q;
    q.push(1, CommandPriority::EMERGENCY);
    q.push(2, CommandPriority::NORMAL);
    q.push(3, CommandPriority::BACKGROUND);
    EXPECT_EQ(q.size(), 3u);
}

TEST(PriorityCommandQueue, EmptyFlag)
{
    PriorityCommandQueue<int> q;
    EXPECT_TRUE(q.empty());
    q.push(1, CommandPriority::NORMAL);
    EXPECT_FALSE(q.empty());
    q.try_pop();
    EXPECT_TRUE(q.empty());
}

TEST(PriorityCommandQueue, CloseUnblocksWaitAndPop)
{
    PriorityCommandQueue<int> q;
    std::thread consumer([&q] { EXPECT_FALSE(q.wait_and_pop().has_value()); });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    q.close();
    consumer.join();
}

TEST(PriorityCommandQueue, CloseAfterPushDrainsAllBuckets)
{
    PriorityCommandQueue<int> q;
    q.push(1, CommandPriority::BACKGROUND);
    q.push(2, CommandPriority::EMERGENCY);
    q.close();
    EXPECT_EQ(*q.wait_and_pop(), 2);  // EMERGENCY first
    EXPECT_EQ(*q.wait_and_pop(), 1);  // BACKGROUND second
    EXPECT_FALSE(q.wait_and_pop().has_value());
}

TEST(PriorityCommandQueue, BucketCountMatchesCommandPriorityRange)
{
    // Ensure kBucketCount covers all CommandPriority values.
    // If a new priority level is added to the enum without updating the queue,
    // this test catches the mismatch.
    EXPECT_EQ(PriorityCommandQueue<int>::kBucketCount, 4u);
}

// ── Parameterised MPSC — different producer/item-count configurations ─────────

struct CmdMpscParams
{
    int producers;
    int items_per_producer;
};

class PriorityCommandQueueMpscTest : public ::testing::TestWithParam<CmdMpscParams>
{
};

TEST_P(PriorityCommandQueueMpscTest, AllItemsDelivered)
{
    const auto [producers, items_per_producer] = GetParam();
    const int total = producers * items_per_producer;

    PriorityCommandQueue<int> q;
    std::vector<std::thread> threads;
    for (int p = 0; p < producers; ++p)
    {
        threads.emplace_back(
            [&q, p, items_per_producer]
            {
                const auto priority = static_cast<CommandPriority>(p % 4);
                for (int i = 0; i < items_per_producer; ++i)
                    q.push(p * items_per_producer + i, priority);
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

INSTANTIATE_TEST_SUITE_P(PriorityCommandQueue, PriorityCommandQueueMpscTest,
                         ::testing::Values(CmdMpscParams{1, 800}, CmdMpscParams{4, 200},
                                           CmdMpscParams{8, 100}),
                         [](const ::testing::TestParamInfo<CmdMpscParams>& info)
                         {
                             return std::to_string(info.param.producers) + "p_" +
                                    std::to_string(info.param.items_per_producer) + "i";
                         });
