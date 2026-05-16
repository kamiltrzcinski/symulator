// tests/server/test_ownership_guard.cpp
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "server/ownership_guard.hpp"

using namespace server;
using engine::core::DispatchAreaID;
using engine::core::PlayerID;

TEST(OwnershipGuard, SetAndCheck)
{
    OwnershipGuard g;
    g.set_owner(DispatchAreaID{"A"}, PlayerID{"P1"});
    EXPECT_TRUE(g.check(DispatchAreaID{"A"}, PlayerID{"P1"}));
    EXPECT_FALSE(g.check(DispatchAreaID{"A"}, PlayerID{"P2"}));
}

TEST(OwnershipGuard, ReleaseOne)
{
    OwnershipGuard g;
    g.set_owner(DispatchAreaID{"A"}, PlayerID{"P1"});
    g.release(DispatchAreaID{"A"});
    EXPECT_FALSE(g.check(DispatchAreaID{"A"}, PlayerID{"P1"}));
    EXPECT_EQ(g.get_owner(DispatchAreaID{"A"}), std::nullopt);
}

TEST(OwnershipGuard, ReleaseAll)
{
    OwnershipGuard g;
    g.set_owner(DispatchAreaID{"A"}, PlayerID{"P1"});
    g.set_owner(DispatchAreaID{"B"}, PlayerID{"P1"});
    g.set_owner(DispatchAreaID{"C"}, PlayerID{"P2"});
    g.release_all(PlayerID{"P1"});
    EXPECT_FALSE(g.check(DispatchAreaID{"A"}, PlayerID{"P1"}));
    EXPECT_FALSE(g.check(DispatchAreaID{"B"}, PlayerID{"P1"}));
    EXPECT_TRUE(g.check(DispatchAreaID{"C"}, PlayerID{"P2"}));
}

TEST(OwnershipGuard, GetOwner)
{
    OwnershipGuard g;
    EXPECT_EQ(g.get_owner(DispatchAreaID{"X"}), std::nullopt);
    g.set_owner(DispatchAreaID{"X"}, PlayerID{"Alice"});
    auto owner = g.get_owner(DispatchAreaID{"X"});
    ASSERT_TRUE(owner.has_value());
    EXPECT_EQ(owner->value, "Alice");
}

TEST(OwnershipGuard, Overwrite)
{
    OwnershipGuard g;
    g.set_owner(DispatchAreaID{"A"}, PlayerID{"P1"});
    g.set_owner(DispatchAreaID{"A"}, PlayerID{"P2"});
    EXPECT_FALSE(g.check(DispatchAreaID{"A"}, PlayerID{"P1"}));
    EXPECT_TRUE(g.check(DispatchAreaID{"A"}, PlayerID{"P2"}));
}

TEST(OwnershipGuard, ConcurrentAccess)
{
    OwnershipGuard g;
    constexpr int kThreads = 8;
    constexpr int kOps = 200;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back(
            [&, i]()
            {
                for (int j = 0; j < kOps; ++j)
                {
                    const std::string area = "AREA-" + std::to_string(j % 5);
                    const std::string pid = "P" + std::to_string(i);
                    g.set_owner(DispatchAreaID{area}, PlayerID{pid});
                    g.check(DispatchAreaID{area}, PlayerID{pid});
                    g.get_owner(DispatchAreaID{area});
                    if (j % 10 == 0)
                        g.release(DispatchAreaID{area});
                }
            });
    }
    for (auto& t : threads)
        t.join();
    // No crash = success
}
