// tests/server/test_ownership_guard.cpp
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "server/ownership_guard.hpp"

using namespace server;
using engine::core::make_uid;
using engine::core::PlayerID;
using engine::core::UID;
using engine::core::UIDDomain;
using engine::core::UIDKind;

// Convenience: make a DISPATCH_AREA UID with a given instance number
static constexpr UID area(std::uint16_t n)
{
    return make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DISPATCH_AREA, 1, n);
}

TEST(OwnershipGuard, SetAndCheck)
{
    OwnershipGuard g;
    g.set_owner(area(1), PlayerID{"P1"});
    EXPECT_TRUE(g.check(area(1), PlayerID{"P1"}));
    EXPECT_FALSE(g.check(area(1), PlayerID{"P2"}));
}

TEST(OwnershipGuard, ReleaseOne)
{
    OwnershipGuard g;
    g.set_owner(area(1), PlayerID{"P1"});
    g.release(area(1));
    EXPECT_FALSE(g.check(area(1), PlayerID{"P1"}));
    EXPECT_EQ(g.get_owner(area(1)), std::nullopt);
}

TEST(OwnershipGuard, ReleaseAll)
{
    OwnershipGuard g;
    g.set_owner(area(1), PlayerID{"P1"});
    g.set_owner(area(2), PlayerID{"P1"});
    g.set_owner(area(3), PlayerID{"P2"});
    g.release_all(PlayerID{"P1"});
    EXPECT_FALSE(g.check(area(1), PlayerID{"P1"}));
    EXPECT_FALSE(g.check(area(2), PlayerID{"P1"}));
    EXPECT_TRUE(g.check(area(3), PlayerID{"P2"}));
}

TEST(OwnershipGuard, GetOwner)
{
    OwnershipGuard g;
    EXPECT_EQ(g.get_owner(area(99)), std::nullopt);
    g.set_owner(area(99), PlayerID{"Alice"});
    auto owner = g.get_owner(area(99));
    ASSERT_TRUE(owner.has_value());
    EXPECT_EQ(owner->value, "Alice");
}

TEST(OwnershipGuard, Overwrite)
{
    OwnershipGuard g;
    g.set_owner(area(1), PlayerID{"P1"});
    g.set_owner(area(1), PlayerID{"P2"});
    EXPECT_FALSE(g.check(area(1), PlayerID{"P1"}));
    EXPECT_TRUE(g.check(area(1), PlayerID{"P2"}));
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
                    const UID a = area(static_cast<std::uint16_t>(1 + j % 5));
                    const std::string pid = "P" + std::to_string(i);
                    g.set_owner(a, PlayerID{pid});
                    g.check(a, PlayerID{pid});
                    g.get_owner(a);
                    if (j % 10 == 0)
                        g.release(a);
                }
            });
    }
    for (auto& t : threads)
        t.join();
    // No crash = success
}
