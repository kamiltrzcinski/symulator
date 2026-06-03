#include <gtest/gtest.h>
#include <engine/core/types.hpp>

using namespace engine::core;

// ── Roundtrip encode/decode ──────────────────────────────────────────────────

TEST(UidCodec, RollingStockCarrierRoundtrip)
{
    auto uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::CARRIER, 0, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::ROLLING_STOCK);
    EXPECT_EQ(uid_kind(uid), UIDKind::CARRIER);
    EXPECT_EQ(uid_scope(uid), 0);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, RollingStockVehicleTypeRoundtrip)
{
    auto uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 0x01B3, 42);
    EXPECT_EQ(uid_domain(uid), UIDDomain::ROLLING_STOCK);
    EXPECT_EQ(uid_kind(uid), UIDKind::VEHICLE_TYPE);
    EXPECT_EQ(uid_scope(uid), 0x01B3);
    EXPECT_EQ(uid_instance(uid), 42);
}

TEST(UidCodec, RollingStockVehicleRoundtrip)
{
    auto uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0x0001, 1000);
    EXPECT_EQ(uid_domain(uid), UIDDomain::ROLLING_STOCK);
    EXPECT_EQ(uid_kind(uid), UIDKind::VEHICLE);
    EXPECT_EQ(uid_scope(uid), 0x0001);
    EXPECT_EQ(uid_instance(uid), 1000);
}

TEST(UidCodec, RollingStockTrainConsistRoundtrip)
{
    auto uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 7);
    EXPECT_EQ(uid_domain(uid), UIDDomain::ROLLING_STOCK);
    EXPECT_EQ(uid_kind(uid), UIDKind::TRAIN_CONSIST);
    EXPECT_EQ(uid_scope(uid), 0);
    EXPECT_EQ(uid_instance(uid), 7);
}

TEST(UidCodec, InfraStationRoundtrip)
{
    // Station GOr: instance 1, scope=1 (station is its own scope)
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::STATION);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, InfraTrackSectionRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 202);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::TRACK_SECTION);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 202);
}

TEST(UidCodec, InfraSwitchRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 2, 5);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::SWITCH);
    EXPECT_EQ(uid_scope(uid), 2);
    EXPECT_EQ(uid_instance(uid), 5);
}

TEST(UidCodec, InfraSignalRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::SIGNAL);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, InfraDerailerRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 3, 2);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::DERAILER);
    EXPECT_EQ(uid_scope(uid), 3);
    EXPECT_EQ(uid_instance(uid), 2);
}

TEST(UidCodec, InfraBlockSectionRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 3);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::BLOCK_SECTION);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 3);
}

TEST(UidCodec, InfraBoundaryNodeRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::BOUNDARY_NODE);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, InfraLevelCrossingRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::LEVEL_CROSSING, 2, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::LEVEL_CROSSING);
    EXPECT_EQ(uid_scope(uid), 2);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, InfraAxleCounterRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 4);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::AXLE_COUNTER);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 4);
}

TEST(UidCodec, InfraInterlockingRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::INTERLOCKING, 3, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::INTERLOCKING);
    EXPECT_EQ(uid_scope(uid), 3);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, InfraPowerSupplyRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::POWER_SUPPLY, 1, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::POWER_SUPPLY);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, InfraDispatchAreaRoundtrip)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DISPATCH_AREA, 3, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::INFRASTRUCTURE);
    EXPECT_EQ(uid_kind(uid), UIDKind::DISPATCH_AREA);
    EXPECT_EQ(uid_scope(uid), 3);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, OpsRouteRoundtrip)
{
    auto uid = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 5);
    EXPECT_EQ(uid_domain(uid), UIDDomain::OPERATIONS);
    EXPECT_EQ(uid_kind(uid), UIDKind::ROUTE);
    EXPECT_EQ(uid_scope(uid), 1);
    EXPECT_EQ(uid_instance(uid), 5);
}

TEST(UidCodec, OpsAlarmRoundtrip)
{
    auto uid = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 0, 1);
    EXPECT_EQ(uid_domain(uid), UIDDomain::OPERATIONS);
    EXPECT_EQ(uid_kind(uid), UIDKind::ALARM);
    EXPECT_EQ(uid_scope(uid), 0);
    EXPECT_EQ(uid_instance(uid), 1);
}

TEST(UidCodec, OpsDispatchExchangeRoundtrip)
{
    auto uid = make_uid(UIDDomain::OPERATIONS, UIDKind::DISPATCH_EXCHANGE, 2, 3);
    EXPECT_EQ(uid_domain(uid), UIDDomain::OPERATIONS);
    EXPECT_EQ(uid_kind(uid), UIDKind::DISPATCH_EXCHANGE);
    EXPECT_EQ(uid_scope(uid), 2);
    EXPECT_EQ(uid_instance(uid), 3);
}

// ── Boundary values ──────────────────────────────────────────────────────────

TEST(UidCodec, MaxScopeAndInstance)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 0xFFFF, 0xFFFF);
    EXPECT_EQ(uid_scope(uid), 0xFFFF);
    EXPECT_EQ(uid_instance(uid), 0xFFFF);
    EXPECT_TRUE(uid_is_safe_json_integer(uid));
}

TEST(UidCodec, MaxDomainAndKind)
{
    // Largest valid combination without exceeding 48 bits
    auto uid = make_uid(UIDDomain::OPERATIONS, UIDKind::DISPATCH_EXCHANGE, 0xFFFF, 0xFFFF);
    EXPECT_EQ(uid_domain(uid), UIDDomain::OPERATIONS);
    EXPECT_EQ(uid_kind(uid), UIDKind::DISPATCH_EXCHANGE);
    EXPECT_TRUE(uid_is_safe_json_integer(uid));
}

// ── JSON safety ───────────────────────────────────────────────────────────────

TEST(UidCodec, AllKindsAreSafeJsonIntegers)
{
    // Max 48-bit UID: domain=0xFF, kind=0xFF, scope=0xFFFF, instance=0xFFFF
    // Bits 47-0 all set = 2^48 - 1 = 0x0000_FFFF_FFFF_FFFF < 2^53
    constexpr std::uint64_t max48 = 0x0000'FFFF'FFFF'FFFFull;
    EXPECT_LT(max48, UID_MAX_SAFE_JSON_INTEGER + 1);

    for (auto [dom, kind] : std::initializer_list<std::pair<UIDDomain, UIDKind>>{
             {UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE},
             {UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE},
             {UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST},
             {UIDDomain::ROLLING_STOCK, UIDKind::CARRIER},
             {UIDDomain::INFRASTRUCTURE, UIDKind::STATION},
             {UIDDomain::INFRASTRUCTURE, UIDKind::DISPATCH_AREA},
             {UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION},
             {UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH},
             {UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL},
             {UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER},
             {UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION},
             {UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE},
             {UIDDomain::INFRASTRUCTURE, UIDKind::LEVEL_CROSSING},
             {UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER},
             {UIDDomain::INFRASTRUCTURE, UIDKind::INTERLOCKING},
             {UIDDomain::INFRASTRUCTURE, UIDKind::POWER_SUPPLY},
             {UIDDomain::OPERATIONS, UIDKind::ROUTE},
             {UIDDomain::OPERATIONS, UIDKind::ALARM},
             {UIDDomain::OPERATIONS, UIDKind::DISPATCH_EXCHANGE},
         })
    {
        auto uid = make_uid(dom, kind, 0xFFFF, 0xFFFF);
        EXPECT_TRUE(uid_is_safe_json_integer(uid))
            << "Not safe JSON integer for domain=" << static_cast<int>(dom)
            << " kind=" << static_cast<int>(kind);
    }
}

// ── No-collision assertions ──────────────────────────────────────────────────

TEST(UidCodec, DifferentKindsDontCollide)
{
    auto a = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
    auto b = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
    auto c = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
    EXPECT_NE(a.value, b.value);
    EXPECT_NE(a.value, c.value);
    EXPECT_NE(b.value, c.value);
}

TEST(UidCodec, DifferentScopesDontCollide)
{
    auto a = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
    auto b = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 2, 1);
    EXPECT_NE(a.value, b.value);
}

TEST(UidCodec, DifferentInstancesDontCollide)
{
    auto a = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
    auto b = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2);
    EXPECT_NE(a.value, b.value);
}

TEST(UidCodec, DifferentDomainsDontCollide)
{
    // ROLLING_STOCK/VEHICLE_TYPE kind=0x01, INFRASTRUCTURE/STATION kind=0x11 — already different
    auto a = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 1, 1);
    auto b = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    EXPECT_NE(a.value, b.value);
}

// ── uid_has_kind helper ──────────────────────────────────────────────────────

TEST(UidCodec, UidHasKindTrue)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
    EXPECT_TRUE(uid_has_kind(uid, UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL));
}

TEST(UidCodec, UidHasKindFalseWrongDomain)
{
    auto uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1);
    EXPECT_FALSE(uid_has_kind(uid, UIDDomain::INFRASTRUCTURE, UIDKind::VEHICLE));
}

TEST(UidCodec, UidHasKindFalseWrongKind)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
    EXPECT_FALSE(uid_has_kind(uid, UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL));
}

// ── Known numeric values (regression) ────────────────────────────────────────

TEST(UidCodec, CarrierInstance1KnownValue)
{
    // Numerically compatible with the old carrier scheme (domain+kind bits unchanged)
    auto uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::CARRIER, 0, 1);
    EXPECT_EQ(uid.value, 0x0000'0104'0000'0001ULL);
}

TEST(UidCodec, StationGOrKnownValue)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    EXPECT_EQ(uid.value, 0x0000'0211'0001'0001ULL);
}

TEST(UidCodec, SignalGOrInstance1KnownValue)
{
    auto uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
    EXPECT_EQ(uid.value, 0x0000'0215'0001'0001ULL);
}
